typedef struct _ColInfo {
	int index;
	udata_t min;
	udata_t max;
} ColInfo;

typedef struct _TableMeta {
	utable_t tid;
	std::vector<int> col_index;
} TableMeta;

typedef struct _JoinData {
	char isDone;
	std::vector<TableMeta> meta;
	std::vector<std::vector<udata_t>> ops;
} JoinData;

typedef struct _TableInfo {
	utable_t tid;
	unumber_t num_key;
	std::vector<ColInfo> col;
	JoinData* join_data;
} TableInfo;

typedef struct _JoinInfo {
	std::pair<utable_t, int> inputR;
	std::pair<utable_t, int> inputS;

} JoinInfo;

//for tree
typedef struct _JoinNode {
	char isDone;
	char isChanged;
	std::vector<int> op_key_position;
	JoinInfo meta;
	struct _JoinNode* inputR;
	JoinData* inputS;
	JoinData output;
} JoinNode;

