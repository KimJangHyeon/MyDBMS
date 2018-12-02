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
	unumber_t num_col;
	std::vector<std::vector<udata_t>> ops;
} JoinData;

typedef struct _JoinNode {
	char isDone;
	//std::vector<std::pair<utable_t, int>> meta;
	std::pair<int, int> virtual_col;
	std::vector<int> virtual_key_position;
	struct _JoinNode* inputR;
	JoinData* inputS;
	JoinData output;


} JoinNode;

