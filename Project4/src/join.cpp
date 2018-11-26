#include <iostream>
#include <algorithm>
#include <iterator>
#include <string>
#include <vector>
#include <memory.h>
#include "types.h"
#include "params.h"
#include "join_struct.h"
#include "views.h"
#include "join.h"


int JoinSet::get_tid_index(utable_t tid, int index, char &isColExist) {
	int ret_index = 0;
	for (std::vector<TableInfo>::iterator iter = table_info.begin(); iter != table_info.end(); ++iter) {
		if (tid != iter->tid) {
			ret_index++;
			continue;
		}
		for (std::vector<ColInfo>::iterator iter1 = iter->col.begin(); iter1 != iter->col.end(); ++iter1) {
				if(iter1->index == index) {
					isColExist = true;
					break;
				}
				isColExist = false;
			}
		return ret_index;
	}
	isColExist = false;
	return -1;
}

//make table_info
void JoinSet::parser(std::string query) {
	int num_join = 0;
	size_t pos = 0;
	std::string token;
	std::vector<std::string> v_join_str;
	std::string delimiter_and = "&";
	std::cout << query << std::endl;	
	while((pos = query.find(delimiter_and)) != std::string::npos) {
		token = query.substr(0, pos);
		//std::cout << token << std::endl;
		v_join_str.push_back(token);
		query.erase(0, pos + delimiter_and.length());
		//std::cout << "query: " + query <<std::endl;
	}
	v_join_str.push_back(query);
	//std::cout << v_join_str.size() << std::endl; 


	JoinInfo join_info;

	for (std::vector<std::string>::iterator iter = v_join_str.begin(); iter != v_join_str.end(); ++iter) {
		utable_t tr, ts;
		int colr, cols;
		std::string delimiter_point = ".";
		std::string delimiter_eq = "=";
		size_t iter_pos = 0;

		//std::cout << "iter "+*iter << std::endl;
		//std::cout << iter->substr(0, iter->find(delimiter_point)) <<std::endl; 
		//std::cout << *iter << std::endl;	

		iter_pos = iter->find(delimiter_point);
		tr = std::stoi(iter->substr(0, iter_pos).c_str());
		//std::cout <<  tr << std::endl;
		iter->erase(0, iter_pos + delimiter_point.length());
		

		iter_pos = iter->find(delimiter_eq);
		colr = std::stoi(iter->substr(0, iter_pos).c_str());
		//std::cout << colr << std::endl;
		iter->erase(0, iter_pos + delimiter_eq.length());

		iter_pos = iter->find(delimiter_point);
		ts = std::stoi(iter->substr(0, iter_pos).c_str());
		//std::cout << ts << std::endl;
		iter->erase(0, iter_pos + delimiter_point.length());
		token = *iter;
		cols = std::stoi(iter->c_str());
		//std::cout << cols << std::endl;	
		cols = std::atoi(iter->c_str());

		join_info.inputR = std::pair<utable_t, int>(tr, colr);
		
		join_info.inputS = std::pair<utable_t, int>(ts, cols);
		this->join_info.push_back(join_info);

		//"========= join info done =========

		int index;
		char isColExist;
		utable_t tid;
		TableInfo table_info;
		ColInfo col_info;
		index = get_tid_index(join_info.inputR.first, join_info.inputR.second - 1, isColExist);	


		if (index == -1) {
			tid = join_info.inputR.first;
			table_info.tid = tid; //join_info.inputR.first;
			col_info.index = join_info.inputR.second - 1;
			col_info.min = CATALOGINITMIN;
			col_info.max = CATALOGINITMAX;
			table_info.col.push_back(col_info);
			
			TableMeta meta;
			meta.tid = tid;

			table_info.join_data = new JoinData;
			table_info.join_data->meta.push_back(meta);
			table_info.join_data->meta[0].col_index.push_back(join_info.inputR.second - 1);

			this->table_info.push_back(table_info);
		}
		else if (index != -1 && !isColExist) {
			tid = join_info.inputR.first;
			table_info.tid = tid;
			col_info.index = join_info.inputR.second - 1;
			col_info.min = CATALOGINITMIN;
			col_info.max = CATALOGINITMAX;
			this->table_info[index].col.push_back(col_info);

			this->table_info[index].join_data->meta[0].col_index.push_back(join_info.inputR.second - 1);
			
		} 
		table_info.col.clear();
		table_info = {};
		index = get_tid_index(join_info.inputS.first, join_info.inputS.second - 1, isColExist);	

		//std::cout << index << std::endl;
		if (index == -1) {
			tid = join_info.inputS.first;
			table_info.tid = tid; //join_info.inputS.first;
			col_info.index = join_info.inputS.second - 1;
			col_info.min = CATALOGINITMIN;
			col_info.max = CATALOGINITMAX;
			table_info.col.push_back(col_info);

			TableMeta meta;
			meta.tid = tid;
			
			table_info.join_data = new JoinData;
			table_info.join_data->meta.push_back(meta);
			table_info.join_data->meta[0].col_index.push_back(join_info.inputS.second - 1);
			
			this->table_info.push_back(table_info);
		}
		else if (index != -1 && !isColExist) {
			col_info.index = join_info.inputS.second - 1;
			col_info.min = CATALOGINITMIN;
			col_info.max = CATALOGINITMAX;
			this->table_info[index].col.push_back(col_info);
		
			this->table_info[index].join_data->meta[0].col_index.push_back(join_info.inputS.second - 1);	
		} 
	}
	
}

bool col_index_cmp(const ColInfo &col1, const ColInfo &col2) {
	return col1.index < col2.index;
}

void JoinSet::scanner() {
	utable_t tid;
	std::vector<ColInfo> cols;
	int num_col = 0;
	udata_t* min_col;
	udata_t* max_col;
	unumber_t num_key = 0;
	int i;

	//get table info, join_data	
	for (std::vector<TableInfo>::iterator iter = this->table_info.begin(); iter != this->table_info.end(); ++iter) {
		cols.clear();
		tid = iter->tid;
		num_col = iter->col.size();
		i = 0;

		sort(iter->join_data->meta[0].col_index.begin(), iter->join_data->meta[0].col_index.end());
		sort(iter->col.begin(), iter->col.end(), col_index_cmp);

		num_key = scan_table(tid, &(iter->col), iter->join_data);
		
		//std::cout << "num key: ";
		//std::cout << num_key << std::endl;
		iter->num_key = num_key;
	}

	//get join ordering
	
}

unumber_t JoinSet::get_num_key(utable_t tid) {
	for (std::vector<TableInfo>::iterator table_iter = this->table_info.begin(); table_iter != this->table_info.end(); ++table_iter) {
		if (table_iter->tid == tid) {
			//std::cout << "tid: " << tid << "num_key: " << table_iter->num_key << std::endl;
			return table_iter->num_key;
		}
	}
	return 0;
}

void JoinSet::order_by_join() {
	std::vector<utable_t> joined_table;
	std::vector<JoinInfo> sorted_join_infos;
	std::vector<std::pair<int, char>> candidate_join;
	std::pair<int, unumber_t> min_cost_join;		//index, joininfo
	unumber_t joined_num_keys;
	unumber_t r_num_key, s_num_key, t_num_key;
	utable_t r_tid, s_tid;
	int r_col, s_col;

	//init min_cost_join
	min_cost_join.first = -1;
	min_cost_join.second = CATALOGINITMIN;

	//get first join target
	int i = 0;
	for (std::vector<JoinInfo>::iterator join_iter = this->join_info.begin(); join_iter != this->join_info.end(); ++join_iter) {
		r_tid = join_iter->inputR.first;
		s_tid = join_iter->inputS.first;
		r_col = join_iter->inputR.second - 1;
		s_col = join_iter->inputS.second - 1;
		r_num_key = get_num_key(r_tid);
		s_num_key = get_num_key(s_tid);

		t_num_key = join_cost(r_num_key, r_tid, r_col, s_num_key, s_tid, s_col);
		if (min_cost_join.second > t_num_key) {
			min_cost_join.first = i;
			min_cost_join.second = t_num_key;
		}
		i++;
	}
	joined_num_keys = min_cost_join.second;

	joined_table.push_back(this->join_info[min_cost_join.first].inputR.first);
	joined_table.push_back(this->join_info[min_cost_join.first].inputS.first);

	sorted_join_infos.push_back(this->join_info[min_cost_join.first]);
	this->join_info.erase(this->join_info.begin() + min_cost_join.first);
	
	//std::cout << min_cost_join.second << ' ' << r_tid << ' ' << r_col << ' ' << std::endl;
	//std::cout << min_cost_join.second << ' ' << s_tid << ' ' << s_col << ' ' << std::endl;
	//std::cout << "===================" << std::endl;


	while(1) { 
	//get candidate
		for (std::vector<JoinInfo>::iterator join_iter = this->join_info.begin(); join_iter != this->join_info.end(); ++join_iter) {
			
			i = 0;
			for (std::vector<utable_t>::iterator table_iter = joined_table.begin(); table_iter != joined_table.end(); ++table_iter) {
				if ( join_iter->inputR.first == (*table_iter)) {
					candidate_join.push_back(std::pair<int, char>(i, 0));
					break;
				} else if (join_iter->inputS.first == (*table_iter)) {
					candidate_join.push_back(std::pair<int, char>(i, 1));
					break;
				}
			}
			i++;
		}

		//loop for compare
		char target;
		//init min_cost_join
		min_cost_join.first = -1;
		min_cost_join.second = CATALOGINITMIN;
		for (std::vector<std::pair<int, char>>::iterator table_iter = candidate_join.begin(); table_iter != candidate_join.end(); ++table_iter) {
			JoinInfo ji = join_info[table_iter->first];
			target = table_iter->second;
			//R is new
			if (target) {
				s_tid = ji.inputR.first;
				s_col = ji.inputR.second - 1;
				s_num_key = get_num_key(s_tid);
				
				r_tid = ji.inputS.first;
				r_col = ji.inputS.second - 1;
				r_num_key = joined_num_keys;
					
				t_num_key = join_cost(r_num_key, r_tid, r_col, s_num_key, s_tid, s_col);
				if (min_cost_join.second > t_num_key) {
					min_cost_join.first = table_iter->first;
					min_cost_join.second = t_num_key;
					//std::cout << t_num_key << ' ' << r_tid << ' ' << r_col << ' ' << std::endl;
					//std::cout << t_num_key << ' ' << s_tid << ' ' << s_col << ' ' << std::endl;
					//std::cout << "===================" << std::endl;
				}

			}
		//S is new
			else {
				s_tid = ji.inputS.first;
				s_col = ji.inputS.second - 1;
				s_num_key = get_num_key(s_tid);
				
				r_tid = ji.inputR.first;
				r_col = ji.inputR.second - 1;
				r_num_key = joined_num_keys;
				std::cout << "joined num keys: " << joined_num_keys << std::endl;
				t_num_key = join_cost(r_num_key, r_tid, r_col, s_num_key, s_tid, s_col);
				if (min_cost_join.second > t_num_key) {
					min_cost_join.first = table_iter->first;
					min_cost_join.second = t_num_key;
					//std::cout << t_num_key << ' ' << r_tid << ' ' << r_col << ' ' << std::endl;
					//std::cout << t_num_key << ' ' << s_tid << ' ' << s_col << ' ' << std::endl;
					//std::cout << "===================" << std::endl;

				}
			}
		}
		joined_num_keys = min_cost_join.second;
		utable_t find_targetR = join_info[min_cost_join.first].inputR.first;	
		utable_t find_targetS = join_info[min_cost_join.first].inputS.first;	
		auto find_resultR = std::find(joined_table.begin(), joined_table.end(), find_targetR);
		auto find_resultS = std::find(joined_table.begin(), joined_table.end(), find_targetS);
		
		if (find_resultR == std::end(joined_table))
			joined_table.push_back(join_info[min_cost_join.first].inputR.first);
		if (find_resultS == std::end(joined_table))
			joined_table.push_back(join_info[min_cost_join.first].inputS.first);

		sorted_join_infos.push_back(join_info[min_cost_join.first]);
		this->join_info.erase(this->join_info.begin() + min_cost_join.first);
	
		if (this->join_info.empty()) {
			break;
			std::cout << "join info is not empty" <<std::endl;
			exit(0);
		}
	}
	this->join_info = sorted_join_infos;
}

unumber_t JoinSet::join_cost(unumber_t num_key1, utable_t tid1, int col1, unumber_t num_key2, utable_t tid2, int col2) {
	unumber_t cost1, cost2;
	unumber_t min1, min2, max1, max2;
	unumber_t intersection_max = CATALOGINITMIN;
	unumber_t intersection_min = CATALOGINITMAX;
	double avg_cost, avg_cost1, avg_cost2;
		

	for (int i = 0; i < this->table_info.size(); i++) {
		if (this->table_info[i].tid == tid1) {
			for (int j = 0; j < this->table_info[i].col.size(); j++) {
				if (this->table_info[i].col[j].index == col1) {
					min1 = this->table_info[i].col[j].min;
					max1 = this->table_info[i].col[j].max;
						
					//std::cout << "min1: " << min1 << std::endl;
					//std::cout << "max1: " << max1 << std::endl;
					if (intersection_min < min1)
						intersection_min = min1;
					if (intersection_max > max1)
						intersection_max = max1;

					//set avg_cost1
					avg_cost1 = ((double)num_key1) / ((double)(max1 - min1));
					//std::cout << "cost1: " << avg_cost1 << std::endl;
				}
			}
		}



		else if (this->table_info[i].tid == tid2) {
			for (int j = 0; j < this->table_info[i].col.size(); j++) {
				if (this->table_info[i].col[j].index == col2) {
					min2 = this->table_info[i].col[j].min;
					max2 = this->table_info[i].col[j].max;
					
					//std::cout << "min2: " << min2 << std::endl;
					//std::cout << "max2: " << max2 << std::endl;
					if (intersection_min < min2)
						intersection_min = min2;
					if (intersection_max > max2)
						intersection_max = max2;
					
					//set avg_cost1
					avg_cost2 = ((double)num_key2) / ((double)(max2 - min2));
					//std::cout << "cost2: " << avg_cost2 << std::endl;
				}
			}
		}

	}
		if (avg_cost1 > avg_cost2) {
			avg_cost = avg_cost2;
			//std::cout <<avg_cost<<std::endl;
		}
		else { 
			avg_cost = avg_cost1;
			//std::cout <<avg_cost<<std::endl;
		}
		//std::cout << num_key1 << ' ' << tid1 << ' ' << col1 << std::endl;
		//std::cout << num_key2 << ' ' << tid2 << ' ' << col2 << std::endl;
		//std::cout << avg_cost <<std::endl;
		//std::cout << intersection_max <<std::endl;
		//std::cout << intersection_min<<std::endl;
		std::cout << "double cost: " << (((double)(intersection_max - intersection_min)) * avg_cost) << std::endl;

		return (unumber_t)(((double)(intersection_max - intersection_min)) * avg_cost); 

}

void JoinSet::join_info_print() {
	for (std::vector<JoinInfo>::iterator iter = this->join_info.begin(); iter != this->join_info.end(); ++iter) {
		std::cout << iter->inputR.first;
		std::cout << iter->inputR.second;
		std::cout << iter->inputS.first;
		std::cout << iter->inputS.second << std::endl;
	}
}

void JoinSet::table_info_print() {
	std::cout << "========================" << std::endl;	
	for (std::vector<TableInfo>::iterator iter = this->table_info.begin(); iter != this->table_info.end(); ++iter) {
		std::cout << "table info" << std::endl;
		std::cout << "tid: ";
		std::cout << iter->tid;
		std::cout << "num key: " << iter->num_key << std::endl;
		std::cout << ": ";
		for (std::vector<ColInfo>::iterator iter1 = iter->col.begin(); iter1 != iter->col.end(); ++iter1) {
			std::cout << iter1->index;
			std::cout << "(";
			std::cout << iter1->min;
			std::cout << ", ";
			std::cout << iter1->max;
			std::cout << ")";
		}
		std::cout << std::endl;
		std::cout <<"--------------------"<<std::endl;
		std::cout << "join data" << std::endl;
		JoinData* join_data = iter->join_data;

		for (std::vector<TableMeta>::iterator table_iter = join_data->meta.begin(); table_iter != join_data->meta.end(); ++table_iter) {
			std::cout << table_iter->tid;
			std::cout << ": ";
			for (int k = 0; k < table_iter->col_index.size(); k++) {
				std::cout << table_iter->col_index[k];
				std::cout << ' ';
			}
			std::cout << std::endl;
		}
		std::cout <<"--------------------"<<std::endl;
		for (std::vector<std::vector<udata_t>>::iterator op_iter = join_data->ops.begin(); op_iter != join_data->ops.end(); ++op_iter) {
			for (std::vector<udata_t>::iterator col_iter = op_iter->begin(); col_iter != op_iter->end(); ++col_iter) {
				std::cout << *col_iter;
				std::cout << ' ';
			}
			std::cout << std::endl;
		
		}
		std::cout << "========================" << std::endl;	
	}
}

//======================== join tree ====================

JoinNode* 
JoinTree::make_node(utable_t r_tid, int r_col, utable_t s_tid, int s_col, JoinNode* join_node, JoinData* join_data) {
	char isTidExist = 0;
	std::vector<TableMeta> check_meta;
	JoinNode* ret_join_node = new JoinNode;
	
	check_meta = join_node->inputR->output.meta;

	for (std::vector<TableMeta>::iterator meta_iter = check_meta.begin(); meta_iter != check_meta.end(); ++meta_iter) {
		if (r_tid == meta_iter->tid)
			isTidExist = 1;
	}
	
	if (isTidExist) {
		ret_join_node->meta.inputR.first = r_tid;
		ret_join_node->meta.inputR.second = r_col;
		ret_join_node->meta.inputS.first = s_tid;
		ret_join_node->meta.inputS.second = s_col;
	} else {
		ret_join_node->meta.inputR.first = s_tid;
		ret_join_node->meta.inputR.second = s_col;
		ret_join_node->meta.inputS.first = r_tid;
		ret_join_node->meta.inputS.second = r_col;
	}
	ret_join_node->inputR = join_node;
	ret_join_node->inputS = join_data;
	ret_join_node->isDone = 0;
	return ret_join_node;
}

JoinNode* 
JoinTree::join_address(int index) {
	return this->join_point[index];
}

int
JoinTree::join_point_size() {
	return this->join_point.size();
}

//1 , 0 1 2 3
//2 , 3 4 5 6
//4 , 0 1 2 3
//4 1

// [9]
// 0 1 2 3 . 0 3 4 5 6 . 0 1 2 3


//return -1 error
int 
JoinTree::get_op_index(JoinData join_data, utable_t tid, int index) {
	int ret = 0;
	std::vector<TableMeta> meta = join_data.meta;
	std::cout << "route" << std::endl;
	for (std::vector<TableMeta>::iterator meta_iter = meta.begin(); meta_iter != meta.end(); ++meta_iter) {
		if(meta_iter->tid != tid) {
			if (meta_iter->col_index[0] == 0) 
				ret += meta_iter->col_index.size();
			else 
				ret += (meta_iter->col_index.size() + 1);
			std::cout << "tid(" << meta_iter->tid << ")\n";
			continue;
		}
		for (std::vector<int>::iterator col_iter = meta_iter->col_index.begin(); col_iter != meta_iter->col_index.end(); ++col_iter) {
			if((*col_iter) != index) {
				std::cout << (*col_iter) << ' ';
				ret++;
				continue;
			}
			std::cout << (*col_iter) << std::endl;
			std::cout << "(success)" << std::endl;
			return ret;
		}
	}
	std::cout << std::endl;
	return -1;
}

JoinData* 
JoinTree::get_join_data(utable_t tid, std::vector<TableInfo> table_info) {
	for (std::vector<TableInfo>::iterator table_info_iter = table_info.begin(); table_info_iter != table_info.end(); ++table_info_iter) {
		if(table_info_iter->tid == tid) {
			return table_info_iter->join_data;
		}
	}
	return NULL;
}

int 
JoinTree::node_meta_size(JoinNode* join_node) {
	int ret = 0;
	for (std::vector<TableMeta>::iterator meta_iter = join_node->output.meta.begin(); meta_iter != join_node->output.meta.end(); ++meta_iter) {
		if(meta_iter->col_index[0] == 0)
			ret += meta_iter->col_index.size();
		else 
			ret += (meta_iter->col_index.size() + 1);
	}
	return ret;
}

void
JoinTree::join(JoinNode* join_node) {
	JoinInfo meta = join_node->meta;
	JoinData r_join_data;
	JoinData s_join_data;
	if (!join_node->isChanged) {
		r_join_data = join_node->inputR->output;
		s_join_data = *(join_node->inputS);
	} else {
		r_join_data = *(join_node->inputS);
		s_join_data = join_node->inputR->output;
	}

	int r_index = get_op_index(r_join_data, meta.inputR.first, meta.inputR.second - 1);
	int s_index = get_op_index(s_join_data, meta.inputS.first, meta.inputS.second - 1);

	std::cout << r_join_data.meta[0].tid << ' ' << meta.inputR.first << ' ' << meta.inputR.second - 1 << std::endl;
	std::cout << s_join_data.meta[0].tid << ' ' << meta.inputS.first << ' ' << meta.inputS.second - 1 << '\n' <<  std::endl;

	std::vector<TableMeta> r_meta = r_join_data.meta;
	std::vector<TableMeta> s_meta = s_join_data.meta;
	std::vector<TableMeta> out_meta = r_meta;
	std::vector<udata_t> out_op;

	//free child node output(because memory)
	if(join_node->inputR != NULL && join_node->inputR->isDone == 1) {
		if(join_node->inputR->inputR != NULL) {
			delete join_node->inputR->inputR;
			join_node->inputR->inputR = NULL;
		}
	}

	if (join_node->inputR != NULL)
		join_node->op_key_position = join_node->inputR->op_key_position;


	char isInclude = 0;
	for (std::vector<TableMeta>::iterator r_meta_iter = r_meta.begin(); r_meta_iter != r_meta.end(); ++r_meta_iter) {
		if(r_meta_iter->tid != s_meta[0].tid) 
			continue;
		isInclude = 1;
	}

	if (!isInclude) { 
		join_node->op_key_position.push_back(node_meta_size(join_node->inputR));
		out_meta.push_back(s_meta[0]);
	}

	join_node->output.meta = out_meta;


	if (r_index == -1 || s_index == -1) {
		std::cout << "error case in get_op_index" << '(' << r_index << ", " << s_index << ')' << std::endl;
		exit(0);
	}

	for (std::vector<std::vector<udata_t>>::iterator r_op_iter = join_node->inputR->output.ops.begin(); r_op_iter != join_node->inputR->output.ops.end(); ++r_op_iter) {
		for (std::vector<std::vector<udata_t>>::iterator s_op_iter = join_node->inputS->ops.begin(); s_op_iter != join_node->inputS->ops.end(); ++s_op_iter) {
			if (r_op_iter[r_index] == s_op_iter[s_index]) {
				//sum two vector
				if (isInclude) {
					join_node->output.ops.push_back(*r_op_iter);
				}
				else {
					out_op = (*r_op_iter);
					for (std::vector<udata_t>::iterator add_op_iter = s_op_iter->begin(); add_op_iter != s_op_iter->end();++add_op_iter) {
						out_op.push_back(*add_op_iter);
					}
				}
				//add to output (op)
				join_node->output.ops.push_back(out_op);
			}
		}
	}
	join_node->isDone = 1;
}

void
JoinTree::make_tree(std::vector<JoinInfo> join_info, std::vector<TableInfo> table_info) {
	JoinNode* r_node = NULL;
	JoinNode* out_node;
	JoinData* r_data;
	JoinData* s_data;
	JoinData* out_data;
	utable_t r_tid, s_tid;
	int r_col, s_col;

	for (std::vector<JoinInfo>::iterator join_info_iter = join_info.begin(); join_info_iter != join_info.end(); ++join_info_iter) {
		r_tid = join_info_iter->inputR.first;
		r_col = join_info_iter->inputR.second;
		s_tid = join_info_iter->inputS.first;
		s_col = join_info_iter->inputS.second;

		if(r_node == NULL) {
			r_node = make_node(r_tid, r_col, s_tid, s_col, NULL, NULL);
			r_data = get_join_data(r_tid, table_info); 
			if (r_data == NULL) {
				std::cout << "get_join_data error(r)" << std::endl;
				exit(0);
			}
			memcpy(&(r_node->output), r_data, sizeof(JoinData));
		}
		s_data = get_join_data(s_tid, table_info);
		if (s_data == NULL) {
			std::cout << "get_join_data error(s)" << std::endl;
			exit(0);
		}
		out_node = make_node(r_tid, r_col, s_tid, s_col, r_node, s_data);
		this->join_point.push_back(out_node);
		r_node = out_node;
	}
	this->header = out_node;
}

void
JoinTree::join_tree_print() {
	utable_t r_tid, s_tid;
	int r_col, s_col;
	JoinData* out_data;
	JoinData* s_data;
	for (std::vector<JoinNode*>::iterator point_iter = join_point.begin(); point_iter != join_point.end(); ++point_iter) {
		r_tid = (*point_iter)->meta.inputR.first;
		s_tid = (*point_iter)->meta.inputS.first;
		r_col = (*point_iter)->meta.inputR.second;
		s_col = (*point_iter)->meta.inputS.second;

		std::cout << "r tid: " << r_tid << std::endl;
		std::cout << "r col: " << r_col << std::endl;
		std::cout << "s tid: " << s_tid << std::endl;
		std::cout << "s col: " << s_col << std::endl;
		std::cout << "-------------------------" << std::endl;
		std::cout << "key position: ";
		for (std::vector<int>::iterator key_iter = (*point_iter)->op_key_position.begin(); key_iter != (*point_iter)->op_key_position.end(); ++key_iter) {
			std::cout << (*key_iter) << ' ';
		}
		std::cout <<std::endl;
		std::cout << "-------------------------" << std::endl;
		std::cout << "inputS: <";
		s_data = (*point_iter)->inputS;
		std::cout << s_data->meta[0].tid << ": ";
		for (std::vector<int>::iterator col_iter = s_data->meta[0].col_index.begin(); col_iter != s_data->meta[0].col_index.end(); ++col_iter) 
			std::cout << (*col_iter) << ' ';
		 std::cout << '>' << std::endl;

		
		for (std::vector<std::vector<udata_t>>::iterator op_iter = s_data->ops.begin(); op_iter != s_data->ops.end(); ++op_iter) {
			for (std::vector<udata_t>::iterator col_iter = op_iter->begin(); col_iter != op_iter->end(); ++col_iter) {
				std::cout << (*col_iter) << ' ';
			}
			std::cout << std::endl;
		}

		std::cout << "-------------------------" << std::endl;
		std::cout << "output: " << std::endl;
		out_data = &((*point_iter)->output);
		for (std::vector<std::vector<udata_t>>::iterator op_iter = out_data->ops.begin(); op_iter != out_data->ops.end(); ++op_iter) {
			for (std::vector<udata_t>::iterator col_iter = op_iter->begin(); col_iter != op_iter->end(); ++col_iter) {
				std::cout << (*col_iter) << ' ';
			}
			std::cout << std::endl;
		}
		std::cout << "=========================" << std::endl;


	}
}
