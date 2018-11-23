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
	std::vector<std::vector<TableMeta>> meta;
	std::vector<std::vector<udata_t>> ops;
} JoinData;

typedef struct _TableInfo {
	unumber_t num_key;
	utable_t tid;
	std::vector<ColInfo> col;
	JoinData* join_data;
} TableInfo;

//for tree
typedef struct _JoinNode {
	struct _JoinNode* inputR;
	JoinData* inputS;
	JoinData output;
} JoinNode;

typedef struct _JoinInfo {
	std::vector<std::pair<utable_t, int>> inputR;
	std::pair<utable_t, int> inputS;

} JoinInfo;

class JoinTree {
	private:
		JoinNode* header;
		std::vector<JoinNode*> join_point;


	public: 

};

//parser -> TableInfo, num_join
//scan -> Joindata
class JoinSet {
	private:
		int num_join;
		std::vector<JoinInfo> join_info; 

	public:
		std::vector<TableInfo> table_info;
		
		void parser(std::string query);		//return num_join//join_info setting + make TableInfo
		void scanner();					//sort<table_info> by join order
		void sort_tables();
		std::vector<JoinInfo> getValidJoin(std::vector<std::pair<utable_t, int>> inputR);	//need for sort_tables

};
