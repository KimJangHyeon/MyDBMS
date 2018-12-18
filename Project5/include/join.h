class JoinTree {
	private:
		JoinNode* header;
		std::vector<JoinNode*> join_point;
		int node_meta_size(JoinNode*);
		JoinNode* make_node(JoinNode* inputR, JoinData* inputS);
		JoinNode* make_leaf_node(JoinData* inputR);
		JoinData* make_data(std::vector<JoinData> join_datas, utable_t tid);
		void setting_node(JoinNode*, JoinInfo);
		bool sum_meta(std::vector<MetaInfo>&, std::vector<MetaInfo>);
		int find_virtual_col(JoinData, utable_t, int);
		void join(JoinNode* join_node);
	public:
		void make_tree(std::vector<JoinInfo>, std::vector<JoinData>);	
		unumber_t join_all();
		void join_data_print(JoinData);
		void join_node_print(JoinNode);
};

class JoinSet {
	private:
		std::vector<std::pair<utable_t, std::vector<int>>> put_join_infos(std::string query);
		void ditect_tid_col(std::vector<std::pair<utable_t, std::vector<int>>>&, utable_t, int);
		void put_join_datas(std::vector<std::pair<utable_t, std::vector<int>>>&);
		void join_ordering();
		std::tuple<double, udata_t, udata_t> get_spectrum(utable_t, int);
		void swap(std::pair<utable_t, int>&, std::pair<utable_t, int>&);
		unumber_t get_joincost(double, udata_t, udata_t, double, udata_t, udata_t);
		std::pair<std::vector<int>, std::vector<JoinInfo>> get_join_candidate(std::vector<utable_t>);
	public:
		std::vector<JoinInfo> join_infos;
		std::vector<JoinData> join_datas;
		void join_infos_print();
		void parser(std::string query);
		void scanner();
		void join_order_print();
};
