class JoinTree {
	private:
		JoinNode* header;
		std::vector<JoinNode*> join_point;
		int node_meta_size(JoinNode*);
	public:
		JoinNode* make_node(utable_t, int, utable_t, int, JoinNode*, JoinData*);
		int get_op_index(JoinData, utable_t, int); //tid, index exception for key
		void make_tree(std::vector<JoinInfo> join_info, std::vector<TableInfo> table_info);
		void join(JoinNode* join_node);
		JoinData* get_join_data(utable_t, std::vector<TableInfo> table_info);
};

//parser -> TableInfo, num_join
//scan -> Joindata
class JoinSet {
	private:
		int num_join;
		int get_tid_index(utable_t, int, char&);
		unumber_t join_cost(unumber_t, utable_t, int, unumber_t, utable_t, int);
		unumber_t get_num_key(utable_t);
	public:
		std::vector<JoinInfo> join_info; 
		std::vector<JoinData> join_data;
		std::vector<TableInfo> table_info;
		void parser(std::string query);		//return num_join//join_info setting + make TableInfo
		void scanner();					//sort<table_info> by join order
		void order_by_join();
		void sort_tables();
		void join_info_print();
		void table_info_print();
		std::vector<JoinInfo> getValidJoin(std::vector<std::pair<utable_t, int>> inputR);	//need for sort_tables

};
