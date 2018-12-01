class JoinTree {
	private:
		JoinNode* header;
		std::vector<JoinNode*> join_point;
		int node_meta_size(JoinNode*);
	public:
		
};

class JoinSet {
	private:
		std::vector<JoinInfo> join_infos;
		std::vector<JoinData> join_datas;
		void put_join_infos(std::string query);
		
	public:
		void join_infos_print();
		void parser(std::string query);
};
