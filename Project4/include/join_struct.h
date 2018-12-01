//parser
//make_tree
typedef struct _JoinInfo {
	std::pair<utable_t, int>[2] input;
} JoinInfos;

typedef struct _MetaInfo {
	utable_t tid;
	int col;
	udata_t min;
	udata_t max;
!} MetaInfo;

typedef struct _JoinData {
	std::vector<MetaInfo> meta;
}


