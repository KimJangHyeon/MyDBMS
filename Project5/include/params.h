
//============= USERSETTING ============
#define INIT_PAGE_NUM	8

#define LRECORD			4//30
#define IRECORD			4//249

#define TABLENAME		32
#define NTABLE			10
#define NTID			10


#define EXTENDPERCENT	0.20
#define MAXTXN			100000

//============= NUMBER ================
//init
#define FREEINIT		(INIT_PAGE_NUM - 1)

//constant
#define NMAXVAL			15
#define CATALOGINITMIN	1844674407370955164
#define CATALOGINITMAX	0
#define MAXINT			2147483647
#define MININT			-2147483648

//============= OFFSET ================
//init
//constant
#define HEADEROFFSET	0
#define INITOFFSET		0

//============= SIZE ==================
#define PAGESIZE		4096
#define PAGEHSIZE		128
#define PAGETHSIZE		64
#define PAGEBHSIZE		64
#define LHEADERSIZE		376
#define IHEADERSIZE		112
#define VALUESIZE		120
#define IRECORDSIZE		16
#define LRECORDSIZE		128

//============= SIGNAL ================
#define TIDFULL			-1
#define FDCLOSE			-1
#define True			1
#define False			0
#define IsKey			-1
#define VUNUSED			1844674407370955164
	//find
#define UPDATE_NO_TREE	1
#define UPDATE_SUCCESS	0
#define UPDATE_KEY_NOTFOUND		2

