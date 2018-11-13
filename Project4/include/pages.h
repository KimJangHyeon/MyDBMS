
typedef struct _L_Record {
    ukey64_t key;
    ukey64_t value[15];
} Lrecord;

typedef struct _I_Record {
    ukey64_t key;
    uoffset_t offset;
} Irecord;

typedef struct _Page {
    ubyte_t byte[PAGESIZE];
} Page;

typedef struct _HeaderPage {
    union {
        struct {
            uoffset_t f_page_offset;
            uoffset_t r_page_offset;
            unumber_t number_of_pages;
            unumber_t number_of_free_pages;
        };
        ubyte_t reserved[PAGESIZE];
    };  
} HeaderPage;

typedef struct _FreePage {
    union {
        uoffset_t next_free_page;
        ubyte_t reserved[PAGESIZE];
    };  
} FreePage;

typedef struct _PageHeaderTop {
	union {
		struct {
			uoffset_t poffset;
			bool isLeaf;
			ukey32_t num_keys;
		};
		ubyte_t reserved[PAGETHSIZE];
	};
} PageHeaderTop;

typedef struct _PageHeaderBottom {
	union {
		struct {

		};
		ubyte_t reserved[PAGEBHSIZE];
	};
} PageHeaderBottom;

typedef struct _NodePage {
	union {
		struct {
			PageHeaderTop header_top;
			PageHeaderBottom header_bottom;
		};
		ubyte_t reserved[PAGESIZE];
	};
} NodePage;

typedef struct _LeafPage {
	union {
		struct {
			union {	
				PageHeaderTop header_top;
				ubyte_t reserved[LHEADERSIZE];
			};											//120
			uoffset_t sibling;							//8
			Lrecord record[LRECORD];					//31 * 128
		};
		ubyte_t bytes[PAGESIZE];
	};
} LeafPage;

typedef struct _InternalPage {
	union {
		struct {
			union {
				PageHeaderTop header_top;
				ubyte_t reserved[IHEADERSIZE];
			};
			Irecord record[IRECORD];
		};
		ubyte_t bytes[PAGESIZE];
	};
} InternalPage;
