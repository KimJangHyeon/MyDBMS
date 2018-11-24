#include <iostream>
#include <algorithm>
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
		std::cout << token << std::endl;
		v_join_str.push_back(token);
		query.erase(0, pos + delimiter_and.length());
		std::cout << "query: " + query <<std::endl;
	}
	v_join_str.push_back(query);
	std::cout << v_join_str.size() << std::endl; 


	JoinInfo join_info;

	for (std::vector<std::string>::iterator iter = v_join_str.begin(); iter != v_join_str.end(); ++iter) {
		utable_t tr, ts;
		int colr, cols;
		std::string delimiter_point = ".";
		std::string delimiter_eq = "=";
		size_t iter_pos = 0;

		std::cout << "iter "+*iter << std::endl;
		//std::cout << iter->substr(0, iter->find(delimiter_point)) <<std::endl; 
		//std::cout << *iter << std::endl;	

		iter_pos = iter->find(delimiter_point);
		tr = std::stoi(iter->substr(0, iter_pos).c_str());
		std::cout <<  tr << std::endl;
		iter->erase(0, iter_pos + delimiter_point.length());
		

		iter_pos = iter->find(delimiter_eq);
		colr = std::stoi(iter->substr(0, iter_pos).c_str());
		std::cout << colr << std::endl;
		iter->erase(0, iter_pos + delimiter_eq.length());

		iter_pos = iter->find(delimiter_point);
		ts = std::stoi(iter->substr(0, iter_pos).c_str());
		std::cout << ts << std::endl;
		iter->erase(0, iter_pos + delimiter_point.length());
		token = *iter;
		cols = std::stoi(iter->c_str());
		std::cout << cols << std::endl;	
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
		std::cout << index << std::endl;
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
	int* para_col;
	udata_t* min_col;
	udata_t* max_col;
	unumber_t num_key = 0;
	int i;
	for (std::vector<TableInfo>::iterator iter = this->table_info.begin(); iter != this->table_info.end(); ++iter) {
		cols.clear();
		tid = iter->tid;
		num_col = iter->col.size();
		para_col = new int[num_col];
		i = 0;

		sort(iter->join_data->meta[0].col_index.begin(), iter->join_data->meta[0].col_index.end());
		sort(iter->col.begin(), iter->col.end(), col_index_cmp);

		num_key = scan_table(tid, iter->col, iter->join_data);
		/*for (std::vector<TableInfo>::iterator col_iter = iter->col.begin(); col_iter != iter->col.end(); ++ col_iter) {
			para_col[i++] = col_iter->index;
		}*/
	}
	
	std::cout << "num key: ";
	std::cout << num_key << std::endl;

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

