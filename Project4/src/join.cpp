#include <iostream>
#include <algorithm>
#include <iterator>
#include <string>
#include <vector>
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
			}
		isColExist = false;
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

