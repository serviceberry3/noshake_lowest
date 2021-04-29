#define _DEFAULT_SOURCE

#include "drm_low.h"

//convenience macros for converting uint_64* to void* and vice versa
#define VOID2U64(x) ((uint64_t)(unsigned long)(x))
#define U642VOID(x) ((void *)(unsigned long)(x))
#define U322VOID(x) ((void*)(unsigned long)(x))

//align a size_t by the page size of the system
#define PGALIGN(x) ((x % getpagesize() != 0) ? (x + (getpagesize() - (x % getpagesize()))) : x)

//memsetting to 0
#define memclear(s) memset(&s, 0, sizeof(s))


//function predeclarations
int sequence_colors_fill_screen();
int sequence_oscillate_square();
void draw_square(int color, int buff_full);


/*for reference
struct drm_mode_card_res {
	__u64 fb_id_ptr;
	__u64 crtc_id_ptr;
	__u64 connector_id_ptr;
	__u64 encoder_id_ptr;
	__u32 count_fbs;
	__u32 count_crtcs;
	__u32 count_connectors;
	__u32 count_encoders;
	__u32 min_width, max_width;
	__u32 min_height, max_height;
};*/

int drmGetCap(int fd, uint64_t capability, uint64_t* value)
{ 
    struct drm_get_cap cap;
    int ret;

    //memclear(cap);
    cap.capability = capability;

    ret = ioctl(fd, DRM_IOCTL_GET_CAP, &cap);
    if (ret!=0)
        return ret;

    *value = cap.value;
    return 0;
}


//array of actual pointers to starts of framebuffers in memory
//each connector could have two dumb buffers for pageflipping, hence the 2d array
void* fb_base[10][2];

//buffer swapper for double buffering implementation
int which_buf = 0;

//the Direct Rendering Infrastructure device file descriptor
int dri_fd;

//array to store lengths of all framebuffers
uint32_t fb_w[10][2];

//array to store widths of all framebuffer
uint32_t fb_h[10][2];

//initialize some structs to NULL
struct drm_mode_crtc crtc = {0};
struct drm_mode_fb_cmd cmd_dumb = {0};
struct drm_mode_fb_cmd cmd_dumb2 = {0};

//instantiate a drm_mode_card_res struct 
struct drm_mode_card_res res = {0};


struct drm_mode_map_dumb map_dumb = {0};
struct drm_mode_map_dumb map_dumb2 = {0};

struct drm_mode_cursor cursor_test;


uint32_t valid_connector;

//reusable iterator
int i;

int drm_setup()
{
	printf("Testing: %d %d %d\n", PGALIGN(245760), PGALIGN(245767), PGALIGN(39698437));

	//return value. Let's HOPE it stays clean
	int ret = 0;

	//TODO: CLEAN UP ON PREMATURE RETURNS

	//kill hwcomposer process so we can get drm master
	system("stop vendor.hwcomposer-2-4");

	//printk("Welcome to drm_low, by Noah Weiner 2020\n");

	//if Xorg is our display server, kill that
	//system("sudo service gdm3 stop");

	//guarantee shutdown and closure of dev file by waiting 1 sec
	usleep(100000);

    //open up the dri device
    dri_fd  = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    if (dri_fd<0) {
        fprintf(stderr, "FAIL on open dev/dri/card0\n");
        return -1;
    }
    printf("Successfully opened dri card0, fd is %d\n", dri_fd);


    //kernel mode setting: set up integer arrays to store IDs of all the diff 4 types of resources
    //frambuffer, CRTC, connector, and encoder
	uint64_t res_fb_buf[10] = {0}, res_crtc_buf[10] = {0}, res_conn_buf[10] = {0}, res_enc_buf[10] = {0};

	//Become the "master" of the DRI device
	if (ioctl(dri_fd, DRM_IOCTL_SET_MASTER, 0) != 0) {
        fprintf(stderr, "Drm set master failed with eror %d: %m\n", errno);
		return errno;
    }
	printf("Drm master success\n");


	//Get resource counts: this fills in count_fbs, count_crtcs, count_connectors, count_encoders
	if (ioctl(dri_fd, DRM_IOCTL_MODE_GETRESOURCES, &res) != 0) {
		fprintf(stderr, "Drm mode getresources failed with error %d: %m\n", errno);
		return errno;
	}
	printf("Drm get resources success\n");


	//allocate memory for buffers containing IDs of the resources
	if (res.count_fbs) {
		res.fb_id_ptr = VOID2U64(calloc(res.count_fbs, sizeof(uint32_t)));
	}
	if (res.count_crtcs) {
		res.crtc_id_ptr = VOID2U64(calloc(res.count_crtcs, sizeof(uint32_t)));
	}
	if (res.count_connectors) {
		res.connector_id_ptr = VOID2U64(calloc(res.count_connectors, sizeof(uint32_t)));
	}
	if (res.count_encoders) {
		res.encoder_id_ptr = VOID2U64(calloc(res.count_encoders, sizeof(uint32_t)));
	}


	//let's check the capabilities of this DRM device to see if it supports DRM dumb buffers
	uint64_t has_dumb;

	if (drmGetCap(dri_fd, DRM_CAP_DUMB_BUFFER, &has_dumb) < 0 || !has_dumb) {
		fprintf(stderr, "DRM device does NOT support dumb buffers\n");
		close(dri_fd);
		return -EOPNOTSUPP;
	}

	printf("Drm device supports dumb buffers\n");


	/*
	//set the id array heads to the ones we created (10 uint64_ts allocated on the stack)
	res.fb_id_ptr = (uint64_t)res_fb_buf;
	res.crtc_id_ptr = (uint64_t)res_crtc_buf;
	res.connector_id_ptr = (uint64_t)res_conn_buf;
	res.encoder_id_ptr = (uint64_t)res_enc_buf;
	*/

	//Get resource IDs by calling get resources a second time
	if (ioctl(dri_fd, DRM_IOCTL_MODE_GETRESOURCES, &res) != 0) {
		fprintf(stderr, "Drm mode getresources second failed with error %d: %m\n", errno);
		return errno;
	}
	printf("Drm get resources second success\n");


	//test
	//printf("%" PRId64 "\n", res.crtc_id_ptr);

	//print out count information
	printf("# framebuffs: %d, # CRTCs: %d, # connectors: %d, # encoders: %d\n", res.count_fbs, res.count_crtcs, res.count_connectors, res.count_encoders);


	/*For reference
	struct drm_mode_get_connector {
	__u64 encoders_ptr;
	__u64 modes_ptr;
	__u64 props_ptr;
	__u64 prop_values_ptr;
	__u32 count_modes;
	__u32 count_props;
	__u32 count_encoders;
	__u32 encoder_id; // Current Encoder 
	__u32 connector_id; // Id 
	__u32 connector_type;
	__u32 connector_type_id;
	__u32 connection;
	__u32 mm_width, mm_height; // HxW in millimeters 
	__u32 subpixel;
	};
	struct drm_mode_modeinfo {
		__u32 clock;
		__u16 hdisplay, hsync_start, hsync_end, htotal, hskew;
		__u16 vdisplay, vsync_start, vsync_end, vtotal, vscan;
		__u32 vrefresh;
		__u32 flags;
		__u32 type;
		char name[DRM_DISPLAY_MODE_LEN];
	};*/

	int found_connected = 0;

	//Loop though all available connectors
	for (i = 0; i < res.count_connectors; i++)
	{
		//instantiate an array of some drm_mode_modeinfo structs to hold the list of modes for this connector
		struct drm_mode_modeinfo conn_mode_buf[20] = {0};

		//arrays to hold the connection properties, conection property values, and encoder IDs for this connector
		uint64_t conn_prop_buf[20] = {0}, conn_propval_buf[20] = {0}, conn_enc_buf[20] = {0};

		//struct for getting the connector info
		struct drm_mode_get_connector conn = {0};

		//get the ID of the current connector we're looking at
		printf("The pointer to the connector ID list is %p, our index is %d, we're looking at address %p\n", (uint64_t*)(U642VOID(res.connector_id_ptr)), i, &((uint64_t*)(U642VOID(res.connector_id_ptr)))[i]);
		conn.connector_id = ((uint32_t*)(U642VOID(res.connector_id_ptr)))[i];

		printf("This connector ID: ");
		printf("%d\n", conn.connector_id);

		//get connector resource counts
		if (ioctl (dri_fd, DRM_IOCTL_MODE_GETCONNECTOR, &conn) != 0) {
			fprintf(stderr, "Drm mode getconnector failed with error %d: %m\n", errno);
			return errno;
		}
		printf("Drm get connector success\n");

		//allocate memories for connector properties and their values
		if (conn.count_props) {
			conn.props_ptr = VOID2U64(calloc(conn.count_props, sizeof(uint32_t)));
			//TODO: add malloc check
			conn.prop_values_ptr = VOID2U64(calloc(conn.count_props, sizeof(uint64_t)));
		}
		else {
			printf("This connector props count is 0\n");
		}

		//allocate memory for connector modes list
		if (conn.count_modes) {
			conn.modes_ptr = VOID2U64(calloc(conn.count_modes, sizeof(struct drm_mode_modeinfo)));
		}
		//TODO: add else check??
		else {
			printf("This connector modes count is 0\n");
		}

		//allocate memory for connector encoders list (list of IDs)
		if (conn.count_encoders) {
			conn.encoders_ptr = VOID2U64(calloc(conn.count_encoders, sizeof(uint32_t)));
		}
		else {
			printf("This connector encoders count is 0\n");
		}

		/*
		conn.modes_ptr=(uint64_t)conn_mode_buf;
		conn.props_ptr=(uint64_t)conn_prop_buf;
		conn.prop_values_ptr=(uint64_t)conn_propval_buf;
		conn.encoders_ptr=(uint64_t)conn_enc_buf;
		*/


		//get actual connector resources
		if (ioctl(dri_fd, DRM_IOCTL_MODE_GETCONNECTOR, &conn) != 0) {
			fprintf(stderr, "Drm mode getconnector second failed with error %d: %m\n", errno);
			return errno;
		}
		printf("Drm get connector second success\n");



		//we know we want to use connector 27 (DSI-1 connector), so OVERRIDE the ID because it was coming back as 0
		if (i == 0) {
			conn.encoder_id = 27;
			((uint32_t*)U642VOID(conn.encoders_ptr))[0] = 27;
		}
		

		//Check if the connector is OK to use (connected to something)
		if (conn.count_encoders < 1 || conn.count_modes < 1 || !conn.encoder_id || !conn.connection) //REDUNDANT
		{
			if (!conn.encoder_id)
				printf("This connections encoder_id field came back empty\n");
			else if (!conn.connection) 
				printf("This connections connection field came back empty\n");

			printf("CONCLUSION: this connector NOT connected\n");

			//this connector not connected, so skip rest of loop, continuing to try next connector
			continue;
		}	

		printf("FOUND a connected connector\n");
		valid_connector = i;

		found_connected = 1;

		/*FOR REFERENCE
		struct drm_mode_create_dumb {
			__u32 height;
			__u32 width;
			__u32 bpp;
			__u32 flags;
			__u32 handle;
			__u32 pitch;
			__u64 size;
		};
		struct drm_mode_fb_cmd {
			__u32 fb_id;
			__u32 width, height;
			__u32 pitch;
			__u32 bpp;
			__u32 depth;
			//driver-specific handle 
			__u32 handle;
		};
		struct drm_mode_map_dumb {
			__u32 handle;
			__u32 pad;
			__u64 offset;
		};*/



		//****MAKE FIRST DUMB BUFFER***************************************************************************************************************************

        //creating dumb buffer
		struct drm_mode_create_dumb create_dumb = {0};
		//struct drm_mode_fb_cmd cmd_dumb = {0};

		memset(&create_dumb, 0, sizeof(create_dumb));
		memset(&cmd_dumb, 0, sizeof(cmd_dumb));
		memset(&map_dumb, 0, sizeof(map_dumb));

		//If we create the buffer later, we can get the size of the screen first.
		//This must be a valid mode, so it's probably best to do this after we find
		//a valid CRTC with modes.

		/*
		create_dumb.width = conn_mode_buf[0].hdisplay;
		create_dumb.height = conn_mode_buf[0].vdisplay;
		*/

		//get the width and height the dumb buffer should be
		create_dumb.width = ((struct drm_mode_modeinfo*)(U642VOID(conn.modes_ptr)))[0].hdisplay * 2;
		create_dumb.height = ((struct drm_mode_modeinfo*)(U642VOID(conn.modes_ptr)))[0].vdisplay * 2;

		printf("create_dumb.width is %d, create_dumb.height is %d\n", create_dumb.width, create_dumb.height);

		//4 bytes per pixel
		create_dumb.bpp = 32;
		create_dumb.flags = 0;

		//these fields filled by kernel during IOCTL_MODE_CREATE_DUMB
		create_dumb.pitch = 0;
		create_dumb.size = 0;
		create_dumb.handle = 0;

		//create the actual dumbbuffer with these characteristics
		if (ioctl(dri_fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_dumb) != 0) {
			fprintf(stderr, "Drm mode create dumb failed with error %d: %m\n", errno);
			return errno;
		}
		printf("IOCTL_MODE_CREATE_DUMB success\n");

		cmd_dumb.width = create_dumb.width;
		cmd_dumb.height = create_dumb.height;
		cmd_dumb.bpp = create_dumb.bpp;
		cmd_dumb.pitch = create_dumb.pitch;
		cmd_dumb.depth = 24;
		cmd_dumb.handle = create_dumb.handle;

		printf("cmd_dumb.width is %d, cmd_dumb.height is %d\n", cmd_dumb.width, cmd_dumb.height);


		//tell the driver to add this dumb buffer as a framebuffer for the display
		if (ioctl(dri_fd, DRM_IOCTL_MODE_ADDFB, &cmd_dumb) != 0) {
			fprintf(stderr, "Drm mode addfb failed with error %d: %m\n", errno);
			return errno;
		}
		printf("IOCTL_MODE_ADDFB success\n");

		map_dumb.handle = create_dumb.handle;

		if (ioctl(dri_fd, DRM_IOCTL_MODE_MAP_DUMB, &map_dumb) !=0) {
			fprintf(stderr, "Drm mode map dumb failed with error %d: %m\n", errno);
			return errno;
		}
		printf("IOCTL_MODE_MAP_DUMB success\n");

		//map some memory for the framebuffer, store the pointer to it in the framebuffer pointers array
		printf("Mapping actual memory using mmap64, requesting %lld bytes, offset is %lld...\n", create_dumb.size, map_dumb.offset);

		//print out size requested with different formatter to verify
		printf("create_dumb.size again: ");
		printf("%" PRId64 "\n", create_dumb.size);

		size_t calculated = (size_t)create_dumb.size;
		printf("Calculated is %d\n", calculated);
		size_t test = PGALIGN(calculated);

		printf("create_dumb.size after x4 and page align: ");
		printf("%d\n", test);

		//1080x2280 is 2462400 pixels for the screen. If each pixel is 4 bytes, that should equate to a create_dumb.size of 9849600, but what we actually see is 9924608. 75008-byte discrepancy
		fb_base[i][0] = (void*)mmap64(0, (size_t)create_dumb.size /*32 bits (4 bytes) per pixel?*/, PROT_READ | PROT_WRITE, MAP_SHARED, dri_fd, map_dumb.offset); 

		if (fb_base[i][0] == MAP_FAILED) {
			fprintf(stderr, "Mmap call failed with error %d: %m\n", errno);
			return errno;
		}

		printf("mmap64 success\n");

		memset(fb_base[i][0], 0, create_dumb.size);

		printf("CHECK #2: create_dumb.width is %d, create_dumb.height is %d\n", create_dumb.width, create_dumb.height);

		//store width and height of the buffer in the framebuffer widths and heights arrays
		fb_w[i][0] = create_dumb.width;
		fb_h[i][0] = create_dumb.height;

		//MAKE SECOND DUMB BUFFER*****************************************************

		//creating dumb buffer
		struct drm_mode_create_dumb create_dumb2 = {0};

		memset(&create_dumb2, 0, sizeof(create_dumb));
		memset(&cmd_dumb2, 0, sizeof(cmd_dumb));
		memset(&map_dumb2, 0, sizeof(map_dumb));

		//If we create the buffer later, we can get the size of the screen first.
		//This must be a valid mode, so it's probably best to do this after we find
		//a valid CRTC with modes.

		/*
		create_dumb.width = conn_mode_buf[0].hdisplay;
		create_dumb.height = conn_mode_buf[0].vdisplay;
		*/

		//get the width and height the dumb buffer should be
		create_dumb2.width = ((struct drm_mode_modeinfo*)(U642VOID(conn.modes_ptr)))[0].hdisplay * 2;
		create_dumb2.height = ((struct drm_mode_modeinfo*)(U642VOID(conn.modes_ptr)))[0].vdisplay * 2;

		printf("create_dumb2.width is %d, create_dumb.height2 is %d\n", create_dumb2.width, create_dumb2.height);

		create_dumb2.bpp = 32;
		create_dumb2.flags = 0;
		create_dumb2.pitch = 0;
		create_dumb2.size = 0;
		create_dumb2.handle = 0;

		//create the actual dumbbuffer with these characteristics
		if (ioctl(dri_fd, DRM_IOCTL_MODE_CREATE_DUMB, &create_dumb2) != 0) {
			fprintf(stderr, "Drm mode create dumb2 failed with error %d: %m\n", errno);
			return errno;
		}
		printf("IOCTL_MODE_CREATE_DUMB2 success\n");

		cmd_dumb2.width = create_dumb2.width;
		cmd_dumb2.height = create_dumb2.height;
		cmd_dumb2.bpp = create_dumb2.bpp;
		cmd_dumb2.pitch = create_dumb2.pitch;
		cmd_dumb2.depth = 24;
		cmd_dumb2.handle = create_dumb2.handle;

		//tell the driver to add this dumb buffer as a framebuffer for the display
		if (ioctl(dri_fd, DRM_IOCTL_MODE_ADDFB, &cmd_dumb2) != 0) {
			fprintf(stderr, "Drm mode addfb2 failed with error %d: %m\n", errno);
			return errno;
		}
		printf("IOCTL_MODE_ADDFB2 success\n");

		map_dumb2.handle = create_dumb2.handle;

		if (ioctl(dri_fd, DRM_IOCTL_MODE_MAP_DUMB, &map_dumb2) !=0) {
			fprintf(stderr, "Drm mode map dumb2 failed with error %d: %m\n", errno);
			return errno;
		}
		printf("IOCTL_MODE_MAP_DUMB2 success\n");

		//map some memory for the framebuffer, store the pointer to it in the framebuffer pointers array
		printf("Mapping actual memory 2 using mmap64, requesting %lld bytes, offset is %lld...\n", create_dumb2.size, map_dumb2.offset);

		//print out size requested with different formatter to verify
		printf("create_dumb2.size again: ");
		printf("%" PRId64 "\n", create_dumb2.size);


		fb_base[i][1] = (void*)mmap64(0, (size_t)create_dumb2.size /*32 bits (4 bytes) per pixel?*/, PROT_READ | PROT_WRITE, MAP_SHARED, dri_fd, map_dumb2.offset); //2462400 * 4?
		printf("dumb buffer 2 memory located at %p, map_dumb2.handle is %p\n", fb_base[i][1], U322VOID(map_dumb2.handle));

		if (fb_base[i][1] == MAP_FAILED) {
			fprintf(stderr, "Mmap call 2 failed with error %d: %m\n", errno);
			return errno;
		}

		printf("mmap64 2 success\n");

		memset(fb_base[i][1], 0, create_dumb2.size);

		printf("CHECK #2: create_dumb2.width is %d, create_dumb.height is %d\n", create_dumb2.width, create_dumb2.height);

		//store width and height of the buffer in the framebuffer widths and heights arrays
		fb_w[i][1] = create_dumb.width;
		fb_h[i][1] = create_dumb.height;

		//fb_base[i][1]+=1;


		/*
		printf("TESTING one more mmap...\n");
		void* test_map = (void*)mmap64(0, (size_t)4096*2423, PROT_READ | PROT_WRITE, MAP_SHARED, dri_fd, 4314816512); 
		if (test_map == MAP_FAILED) {
			fprintf(stderr, "Mmap call 3 test failed with error %d: %m\n", errno);
			return errno;
		}
		printf("mmap64 3 test success\n");*/


		//SET UP ENCODER AND CRTC

        //kernel mode
		printf("Connection #%d: # modes: %d, # props: %d, # encoders: %d\n", conn.connection, conn.count_modes, conn.count_props, conn.count_encoders);

		printf("Modes w x h from struct drm_mode_modeinfo: %d x %d, address in memory where dumb buffer starts: %p\n", ((struct drm_mode_modeinfo*)(U642VOID(conn.modes_ptr)))[0].hdisplay, ((struct drm_mode_modeinfo*)(U642VOID(conn.modes_ptr)))[0].vdisplay, fb_base[i]);

		/*FOR REFERENCE
		struct drm_mode_get_encoder {
			__u32 encoder_id;
			__u32 encoder_type;
			__u32 crtc_id; // Id of crtc
			__u32 possible_crtcs;
			__u32 possible_clones;
		};*/

		//instantiate a drm_mode_get_encoder struct
		struct drm_mode_get_encoder enc = {0};

		//we already know the encoder ID
		enc.encoder_id = conn.encoder_id;
		printf("Already have the encoder ID for this connector, which is %d\n", enc.encoder_id);

	
		//get more information about the encoder for this connector
		if (ioctl(dri_fd, DRM_IOCTL_MODE_GETENCODER, &enc) != 0) {
			fprintf(stderr, "Drm mode getencoder failed with error %d: %m\n", errno);
			return errno;
		}
		printf("IOCTL_MODE_GETENCODER success\n");

		/*FOR REFERENCE
		struct drm_mode_crtc {
			__u64 set_connectors_ptr;
			__u32 count_connectors;
			__u32 crtc_id; //Id 
			__u32 fb_id; //Id of framebuffer 
			__u32 x, y; //Position on the framebuffer
			__u32 gamma_size;
			__u32 mode_valid;
			struct drm_mode_modeinfo mode;
		};*/

		//struct drm_mode_crtc crtc = {0};

		//already should have the ID number of the CRTC for this encoder
		//crtc.crtc_id = enc.crtc_id;

		//OVERRIDE CRTC_ID NUMBER
		crtc.crtc_id = 125;  //127-0 or 181-1?
		printf("Already have the CRTC ID for this encoder, which is %d\n", crtc.crtc_id);

		//get more info about the CRTC for this encoder, filling in the drm_mode_crtc struct 
		if (ioctl(dri_fd, DRM_IOCTL_MODE_GETCRTC, &crtc) != 0) {
			fprintf(stderr, "Drm mode get CRTC failed with error %d: %m\n", errno);
			return errno;
		}
		printf("IOCTL_MODE_GETCRTC success\n");

		//set up the CRTC
		crtc.fb_id = cmd_dumb.fb_id;
		printf("Already have the dumb FB ID for this encoder, which is %d\n", crtc.fb_id);

		//crtc.set_connectors_ptr = (uint64_t)&res_conn_buf[i];

		crtc.set_connectors_ptr = VOID2U64( (void*) &((uint32_t*)(U642VOID(res.connector_id_ptr)))[i] );
		printf("crtc.set_connectors_ptr is %p\n", U642VOID(crtc.set_connectors_ptr));

		crtc.count_connectors = 1;

		//crtc.mode = conn_mode_buf[0];

		crtc.mode = ((struct drm_mode_modeinfo*)(U642VOID(conn.modes_ptr)))[0];

		//mode_valid has to be set to true; otherwise driver won't do anything
		crtc.mode_valid = 1;

		crtc.x = 540;
		crtc.y = 1140;

		//Connect the CRTC to our newly created dumb buffer
		if (ioctl(dri_fd, DRM_IOCTL_MODE_SETCRTC, &crtc) != 0) {
			fprintf(stderr, "Drm mode set CRTC failed with error %d: %m\n", errno);
			return errno;
		}
		printf("IOCTL_MODE_SETCRTC success\n");
	}

	//PROBLEM
	if (!found_connected) {
		printf("No connected connectors found\n");
		//fprintf(stderr, "ERROR: No connected connectors found\n");
		return -1;
	}

	/*OPTIONAL
	//Stop being the "master" of the DRI device - DROP MASTER
	if (ioctl(dri_fd, DRM_IOCTL_DROP_MASTER, 0) !=0) {
		fprintf(stderr, "Drm drop master failed with error %d: %m\n", errno);
		return errno;
	}
	printf("Drm drop master success\n");
	*/

	//pick a sequence to run now
	//ret = sequence_colors_fill_screen();
	//ret = sequence_oscillate_square();

	//draw static image once
	draw_square(0x00000000, 1);

	//display the back buffer
	crtc.fb_id = cmd_dumb2.fb_id;

	//Connect the CRTC to our newly created dumb buffer
	if (ioctl(dri_fd, DRM_IOCTL_MODE_SETCRTC, &crtc) != 0) {
		fprintf(stderr, "Drm mode set CRTC failed with error %d: %m\n", errno);
		return errno;
	}
	printf("IOCTL_MODE_SETCRTC success\n");

	//Bounce the square back and forth across the screen for 15 seconds.

	printf("drm_setup() DONE\n");

	return ret;
}


int close_dri() {
	//close the DRI dev
	close(dri_fd);

	return 0;
}

//display 100 seconds of beautiful colors filling the screen
int sequence_colors_fill_screen() {
	//iterators
	int x, y, oscillator, direction = 0;

	printf("Starting iterations for writing to FB...\n");
	for (i = 0; i < 100; i++)
	{
		int j;
		void* current_page;


		//iterate over all connectors
		for (j = 0; j < res.count_connectors; j++)
		{
			if (j != valid_connector)
				continue;


			//double buffering
			if (which_buf) {
				current_page = fb_base[j][0];
			}
			else {
				current_page = fb_base[j][1];
			}
			
			printf("Coloring for connector #%d\n", j);

			//select random color
			printf("Selecting color...\n");
			int color = (rand() % 0x00ffffff) & 0x00ff00ff;
			printf("Color is %d\n", color);


			//for all rows of pixels
			printf("Color selection done, starting coloring double-loop...\n");

			printf("fb_h[j] reads %d, fb_w[j] read %d\n", fb_h[j][0], fb_w[j][0]);

			for (y = 0; y < fb_h[j][0]; y++) {
			
				//printf("Row number %d\n", y);
				//for all pixels in the row
				for (x = 0; x < fb_w[j][0]; x++)
				{
					//printf("Calculating pixel location, column number %d\n", x);
					//calculate offset into framebuffer memory for this pixel
					int location = (y * (fb_w[j][0])) + x; // = ((which row # we're on * width of the rows) + which column # we're on)

					//printf("Setting pixel to color...\n");
					//set this pixel to the color
					*(((uint32_t*) current_page) + location) = color;
				}
			}

			//double buffering
			if (which_buf) {
				crtc.fb_id = cmd_dumb.fb_id;
				which_buf = 0;
			}
			else {
				crtc.fb_id = cmd_dumb2.fb_id;
				which_buf = 1;
			}

			//Connect the CRTC to the correct page (back page)
			if (ioctl(dri_fd, DRM_IOCTL_MODE_SETCRTC, &crtc) !=0) {
				fprintf(stderr, "Drm mode set CRTC failed with error %d: %m\n", errno);
				return errno;
			}
			printf("IOCTL_MODE_SETCRTC success\n");
				

			printf("Connector #%d done\n", j);
		}

		//sleep for 1 million microseconds = 1 sec = 1000ms between each color
		usleep(1000000);

		printf("Color #%d done\n", i);
	}
	return 0;
}


int adjust_crtc_offset(float x_off, float y_off) {
	//make sure to use back buffer
	crtc.fb_id = cmd_dumb2.fb_id;

	crtc.x += (__u32) x_off;
	crtc.y += (__u32) y_off;

	//Connect the CRTC to the correct page (back page)
	if (ioctl(dri_fd, DRM_IOCTL_MODE_SETCRTC, &crtc) != 0) {
		fprintf(stderr, "Drm mode set CRTC failed with error %d: %m\n", errno);
		return errno;
	}
}

//make a red square bounce back and forth between the edges of the screen for 30 seconds
int sequence_oscillate_square() {
	//iterators
	int j, k;

	//draw static image once
	draw_square(0x00000000, 1);

	//Bounce the square back and forth across the screen for 15 seconds.

	//use back buffer
	crtc.fb_id = cmd_dumb2.fb_id;

	//outer loop for convenience
	for (i = 0; i < 1; i++) {
		double time_taken = 0;
		clock_t t;

		//move square to left of screen
		for (j = 0; j < 20; j++) {
			/*
			//double buffering - unnecessary here since we're not rerendering
			if (which_buf) {
				crtc.fb_id = cmd_dumb.fb_id;
				which_buf = 0;
			}
			else {
				crtc.fb_id = cmd_dumb2.fb_id;
				which_buf = 1;
			}*/

			//move square to left side of screen by increasing the x index into the framebuffer and resetting the CRTC
			for (k = 0; k < 9; k++) {
				t = clock(); 
			
				crtc.x += 80;

				//Connect the CRTC to the correct page (back page)
				if (ioctl(dri_fd, DRM_IOCTL_MODE_SETCRTC, &crtc) != 0) {
					fprintf(stderr, "Drm mode set CRTC failed with error %d: %m\n", errno);
					return errno;
				}

				t = clock() - t; 

				time_taken += (double)t; // in seconds 
			}


			//move square back to right side of screen
			for (k = 0; k < 9; k++) {
				t = clock();

				crtc.x -= 80;

				//Connect the CRTC to the correct page (back page)
				if (ioctl(dri_fd, DRM_IOCTL_MODE_SETCRTC, &crtc) !=0) {
					fprintf(stderr, "Drm mode set CRTC failed with error %d: %m\n", errno);
					return errno;
				}

				t = clock() - t;

				time_taken += (double)t;
			}
		}

		//print out average speed of a move (should be ~73 microseconds)
		printf("Averaged %f seconds per move\n", time_taken / 360.0 / CLOCKS_PER_SEC);
	}
	return 0;
}


//draw a square on the right-hand side of the screen
void draw_square(int color, int buff_full) {
	//iterators
	int j, x, y;

	void* current_page;
	int white, square;

	int x_width;

	int off = 0;


	//iterate over all of our connectors (should just be one)
	for (j = 0; j < res.count_connectors; j++)
	{
		//make sure this is a valid connector
		if (j != valid_connector)
			continue;

		if (which_buf) {
			current_page = fb_base[j][0];
		}
		else {
			current_page = fb_base[j][1];
		}

		//if buff_full is 1, use entire buffer width (twice the width of the screen), otherwise just use half
		x_width = (buff_full) ? fb_w[j][0] : fb_w[j][0] / 2;

		printf("Coloring for connector #%d\n", j);

		//set color values
		square = color;
		white = 0xFFFFFFFF;


		printf("Color selection done, starting coloring loops...\n");
		printf("fb_h[j] reads %d, fb_w[j] read %d\n", fb_h[j][0], fb_w[j][0]);

		int off = 0;
		
		
		//DRAW BLACK SQUARE ON WHITE BACKGROUND - ONLY ON TOP-LEFT QUARTER OF FRAMEBUFFER


		//for all rows of pixels
		for (y = 0; y < fb_h[j][0]; y++) { //iterates ~4560 times   //fb_h[j][0] / 2
		
			if (y > 2080 && y < 2480) {
			//printf("Row number %d\n", y);
			//for all pixels in the row
				for (x = 0; x < x_width; x++) //iterates ~2160 times
				{
						if (x > 880 && x < 1280) {
							//printf("Calculating pixel location, column number %d\n", x);
							//calculate offset into framebuffer memory for this pixel
							//note: need to skip over extra pad on side using off
							int location = (y * fb_w[j][0] + off) + x; //fb_w[j][0]

							//printf("Setting pixel to color...\n");
							//set this pixel to the color
							*(((uint32_t*) current_page) + location) = square;
						
						}

						else {
							//printf("Calculating pixel location, column number %d\n", x);
							//calculate offset into framebuffer memory for this pixel
							//note: need to skip over extra pad on side using off
							int location = (y * fb_w[j][0] + off) + x; //fb_w[j][0]

							//printf("Setting pixel to color...\n");
							//set this pixel to the color
							*(((uint32_t*) current_page) + location) = white;
						}
				}
			}


			//for rows outside of the square, apply white
			else {
				for (x = 0; x < x_width; x++) //iterates ~2160 times
				{
				
					//calculate offset into framebuffer memory for this pixel
					//note: need to skip over extra pad on side using off
					int location = (y * fb_w[j][0] + off) + x; //fb_w[j][0]

					//printf("Setting pixel to color...\n");
					//set this pixel to the color
					*(((uint32_t*) current_page) + location) = white;
				}
			}


			//apparently the fb isn't actually 1080 x 2280 :)
			off += 16;
		}


	}

	printf("Finished draw_square()\n");
}