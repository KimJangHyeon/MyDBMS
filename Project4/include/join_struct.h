//parser
//make_tree
typedef struct _JoinInfo {
	std::pair<utable_t, int> input[2];
} JoinInfo;

typedef struct _MetaInfo {
	utable_t tid;
	int col;
	udata_t min;
	udata_t max;
} MetaInfo;

typedef struct _JoinData {
	std::vector<MetaInfo> meta;
} JoinData;

typedef struct _JoinNode {
	char isDone;
	JoinInfo join_target;
	//HERE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	std::vector<int> key_position;
	JoinData* inputR;
	JoinData* inputS;
	JoinData* output;


} JoinNode;

