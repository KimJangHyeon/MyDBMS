#include <iostream>
#include <algorithm>
#include <iterator>
#include <string>
#include <cstring>
#include <vector>
#include <memory>
#include <tuple>
#include <stdexcept>

#include "types.h"
#include "params.h"
#include "pages.h"
#include "join_struct.h"
#include "views.h"
#include "join.h"

bool 
table_cmp(const std::pair<utable_t, std::vector<int>> &a, const std::pair<utable_t, std::vector<int>> &b) {
	return a.first < b.first;
}

 

void
JoinSet::ditect_tid_col(std::vector<std::pair<utable_t, std::vector<int>>> &ret, utable_t t, int col) {
		//get which tid, col appeared
		char isTidExist = 0;
		char isColExist = 0;
		int tid_index = 0;
		for (std::vector<std::pair<utable_t, std::vector<int>>>::iterator ret_iter = ret.begin();
				ret_iter != ret.end(); ++ret_iter) {
			if (ret_iter->first != t) {
				tid_index++;
				continue;
			}
			isTidExist = 1;
			for (std::vector<int>::iterator col_iter = ret_iter->second.begin();
					col_iter != ret_iter->second.end(); ++col_iter) {
				if ((*col_iter) != col)
					continue;
				isColExist = 1;
			}
			break;
		}
		if (!isTidExist) {
			std::pair<utable_t, std::vector<int>> target;
			target.first = t;
			target.second.push_back(col);
			ret.push_back(target);
		} else if (!isColExist) {
			ret[tid_index].second.push_back(col);
		}
}


std::vector<std::pair<utable_t, std::vector<int>>>
JoinSet::put_join_infos(std::string query) {
	std::vector<std::pair<utable_t, std::vector<int>>> ret;
	int pos = 0;
	std::string token;
	std::vector<std::string> v_join_str;
	std::string delimiter[3] = { "&", ".", "=" };
	JoinInfo join_info;
	
	//parse per join
	while((pos = query.find(delimiter[0])) != std::string::npos) {
		token = query.substr(0, pos);
		v_join_str.push_back(token);
		query.erase(0, pos + delimiter[0].length());
	}
	v_join_str.push_back(query);

	utable_t t[2];
	int col[2];
	int iter_pos = 0;
	for (std::vector<std::string>::iterator join_str_iter = v_join_str.begin();
			join_str_iter != v_join_str.end(); 
			++join_str_iter) {
		
		for (int i = 0; i < 2; i++) {
			iter_pos = join_str_iter->find(delimiter[1]);
			t[i] = std::stoi(join_str_iter->substr(0, iter_pos).c_str());
			join_str_iter->erase(0, iter_pos + delimiter[1].length());
			if (iter_pos != -1) {
				iter_pos = join_str_iter->find(delimiter[2]);
				col[i] = std::stoi(join_str_iter->substr(0, iter_pos).c_str());
				join_str_iter->erase(0, iter_pos + delimiter[2].length());
			} else {
				col[i] = std::stoi(join_str_iter->c_str());
				col[i] = std::atoi(join_str_iter->c_str());
			}
		}

		for (int i = 0; i < 2; i++) {
			ditect_tid_col(ret, t[i], col[i]);
			join_info.input[i] = std::pair<utable_t, int>(t[i], col[i]);
		}
		this->join_infos.push_back(join_info);
	}
	return ret;
}

void
JoinSet::put_join_datas(std::vector<std::pair<utable_t, std::vector<int>>> &join_tid_col) {
	std::pair<utable_t, std::vector<int>> t_tid_col;
	JoinData jd;
	MetaInfo mi;
	while(!join_tid_col.empty()) {
		if (join_tid_col[0].second[0] != 1) 
			join_tid_col[0].second.insert(join_tid_col[0].second.begin(), 1);
		t_tid_col = join_tid_col[0];
		jd.num_col = 0;
		
		for (std::vector<int>::iterator col_iter = t_tid_col.second.begin();
				col_iter != t_tid_col.second.end(); ++col_iter) {
			mi.tid = t_tid_col.first;
			mi.col = *col_iter;
			mi.min = CATALOGINITMIN;
			mi.max = CATALOGINITMAX;
			jd.meta.push_back(mi);
		}

		this->join_datas.push_back(jd);
		join_tid_col.erase(join_tid_col.begin());
		jd.meta.clear();
	}
}

void 
JoinSet::parser(std::string query) {
	std::vector<std::pair<utable_t, std::vector<int>>> join_tid_col;
	join_tid_col = put_join_infos(query);
	
	sort(join_tid_col.begin(), join_tid_col.end(), table_cmp);
	for (std::vector<std::pair<utable_t, std::vector<int>>>::iterator col_iter = join_tid_col.begin();
			col_iter != join_tid_col.end(); ++col_iter) {
		sort(col_iter->second.begin(), col_iter->second.end());
	}
	put_join_datas(join_tid_col);
	

}
std::tuple<double, udata_t, udata_t>
JoinSet::get_spectrum(utable_t tid, int col) {
	ukey64_t num_keys;
	udata_t min;
	udata_t max;
	double avg_cost;
	for (std::vector<JoinData>::iterator data_iter = join_datas.begin(); 
			data_iter != join_datas.end(); ++data_iter) {
		if (data_iter->meta[0].tid != tid)
			continue;
		num_keys = data_iter->num_col;

		for (std::vector<MetaInfo>::iterator meta_iter = data_iter->meta.begin();
				meta_iter != data_iter->meta.end(); ++meta_iter) {
			if (meta_iter->col != col) 
				continue;

			min = meta_iter->min;
			max = meta_iter->max;
			if (max == min)
				avg_cost = num_keys;
			else
				avg_cost = (double)(num_keys/(max - min));
		}
	}
	return std::make_tuple(avg_cost, min, max);
}

void
JoinSet::swap(std::pair<utable_t, int>& input1, std::pair<utable_t, int>& input2) {
	std::pair<utable_t, int> temp;
	temp = input1;
	input1 = input2;
	input2 = temp;
}

unumber_t
JoinSet::get_joincost(double avg_cost1, udata_t min1, udata_t max1, double avg_cost2, udata_t min2, udata_t max2) {
	double avg_cost = (avg_cost1 >avg_cost2) ? avg_cost2: avg_cost1;
	udata_t min = (min1 > min2) ? min1 : min2;
	udata_t max = (max1 > max2) ? max2 : max1;

	if (max == min)
		return avg_cost;

	return (unumber_t)(avg_cost * (max - min));
}

bool
table_check_and_push(std::vector<utable_t> &cur_tids, utable_t tid) {
	std::vector<utable_t>::iterator iter;
	iter = find(cur_tids.begin(), cur_tids.end(), tid);
	if (iter == cur_tids.end()) {
		cur_tids.push_back(tid);
		return 0;
	}
	else 
		return 1;
	
}

std::pair<std::vector<int>, std::vector<JoinInfo>>
JoinSet::get_join_candidate(std::vector<utable_t> cur_tids) {
	std::pair<std::vector<int>, std::vector<JoinInfo>> candidate_join;
	int i = 0;
	for (std::vector<JoinInfo>::iterator join_infos_iter = this->join_infos.begin(); 
			join_infos_iter != this->join_infos.end(); ++join_infos_iter) {
		
		for (std::vector<utable_t>::iterator table_iter = cur_tids.begin();
				table_iter != cur_tids.end(); ++table_iter) {

			if ((*table_iter) == join_infos_iter->input[0].first) {
				candidate_join.second.push_back(*join_infos_iter);
				candidate_join.first.push_back(i);
				break;
			}
			else if ((*table_iter) == join_infos_iter->input[1].first) {
				swap(join_infos_iter->input[0], join_infos_iter->input[1]);
				candidate_join.second.push_back(*join_infos_iter);
				candidate_join.first.push_back(i);
				break;
			}
		}
		i++;
	}

	return candidate_join;
}

void
JoinSet::join_ordering() {
	//get first

	std::vector<utable_t> cur_tids;
	unumber_t cur_join_cost = CATALOGINITMIN;
	int cur_index;
	std::vector<JoinInfo> ordered_join;

	unumber_t cmp_cost;
	utable_t r_tid, s_tid;
	int r_col, s_col;
	udata_t r_min, s_min, r_max, s_max;
	double avg1, avg2;
	std::pair<std::vector<int>, std::vector<JoinInfo>> candidate_join;
	cur_tids.clear();

	//set first join
	if (cur_tids.empty()) {
		for (int i = 0; i < join_infos.size(); i++) {
			r_tid = join_infos[i].input[0].first;
			s_tid = join_infos[i].input[1].first;

			r_col = join_infos[i].input[0].second;
			s_col = join_infos[i].input[1].second;

			std::tie(avg1, r_min, r_max) = get_spectrum(r_tid, r_col);
			std::tie(avg2, s_min, s_max) = get_spectrum(s_tid, s_col);

			cmp_cost = get_joincost(avg1, r_min, r_max, avg2, s_min, s_max);
			
			if (cur_join_cost > cmp_cost) {
				cur_join_cost = cmp_cost;
				cur_index = i;
				///pop -> front push
			}
		}
		table_check_and_push(cur_tids, this->join_infos[cur_index].input[0].first);
		table_check_and_push(cur_tids, this->join_infos[cur_index].input[1].first);

		ordered_join.push_back(this->join_infos[cur_index]);
		this->join_infos.erase(this->join_infos.begin() + cur_index);
	}

	while(!join_infos.empty()) {
		candidate_join.first.clear();
		candidate_join.second.clear();
		candidate_join = get_join_candidate(cur_tids);
		
		for (int i = 0; i < candidate_join.second.size(); i++) {
			JoinInfo* candidate_iter = &(candidate_join.second[i]);
			int candidate_index = candidate_join.first[i];
		
			std::tie(avg1, r_min, r_max) = get_spectrum(candidate_iter->input[0].first, candidate_iter->input[0].second);
			avg1 = (double)cur_join_cost/(r_max - r_min);
			std::tie(avg2, s_min, s_max) = get_spectrum(candidate_iter->input[1].first, candidate_iter->input[1].second);
			
			cmp_cost = get_joincost(avg1, r_min, r_max, avg2, s_min, s_max);

			if (cur_join_cost >= cmp_cost) {
				cur_join_cost = cmp_cost;
				cur_index = candidate_index;
			}
		}

		table_check_and_push(cur_tids, this->join_infos[cur_index].input[0].first);
		table_check_and_push(cur_tids, this->join_infos[cur_index].input[1].first);

		ordered_join.push_back(this->join_infos[cur_index]);
		this->join_infos.erase(this->join_infos.begin() + cur_index);
	}

	this->join_infos.clear();
	this->join_infos = ordered_join;
}

void
JoinSet::scanner() {

	for (std::vector<JoinData>::iterator jd_iter = this->join_datas.begin(); 
			jd_iter != this->join_datas.end(); ++jd_iter) {
		scan_table((*jd_iter));
	}
	join_ordering();
	int i =0;
}

void
JoinSet::join_infos_print() {
	JoinInfo ji;
	for (std::vector<JoinInfo>::iterator iter = this->join_infos.begin(); 
			iter != this->join_infos.end(); ++iter) {
		ji = (*iter);
		std::cout << "r tid: " << ji.input[0].first << " r col: " << ji.input[0].second << std::endl;
		std::cout << "s tid: " << ji.input[1].first << " s col: " << ji.input[1].second << std::endl;
		std::cout << "-------------------------------" << std::endl;
	}
}

void
JoinSet::join_order_print() {
	for (int i = 0; i < join_infos.size(); i++) {
		std::cout << '(' << join_infos[i].input[0].first << ", " << join_infos[i].input[0].second << " / "
			<< join_infos[i].input[1].first << ", " << join_infos[i].input[1].second << ')' << std::endl;
	}
	std::cout << "==============================" << std::endl;
}
//===============================================================================================

JoinNode* 
JoinTree::make_node(JoinNode* inputR, JoinData* inputS) {
	JoinNode* ret = new JoinNode;
	ret->isDone = 0;
	ret->inputR = inputR;
	ret->inputS = inputS;

	return ret;
}

JoinNode*
JoinTree::make_leaf_node(JoinData* inputR) {
	JoinNode* ret = new JoinNode;
	ret->isDone = 1;
	ret->inputR = NULL;
	ret->inputS = NULL;
	memcpy(&ret->output, inputR, sizeof(JoinData));
	ret->virtual_key_position.push_back(0);
	return ret;
}

JoinData*
JoinTree::make_data(std::vector<JoinData> join_datas, utable_t tid) {
	JoinData* ret = new JoinData;
	for (std::vector<JoinData>::iterator data_iter = join_datas.begin();
			data_iter != join_datas.end(); ++data_iter) {
		if (data_iter->meta[0].tid != tid)
			continue;
		//memcpy(ret, &(*data_iter), sizeof(JoinData));
		for (int i = 0; i < data_iter->meta.size(); i++) {
			ret->meta.push_back(data_iter->meta[i]);
		}
		ret->num_col = data_iter->num_col;
		ret->ops = data_iter->ops;
	}
	return ret;
}

bool
JoinTree::sum_meta(std::vector<MetaInfo>& v1, std::vector<MetaInfo> v2) {
	bool isInclude = 0;
	for (std::vector<MetaInfo>::iterator meta_iter1 = v1.begin(); 
			meta_iter1 != v1.end(); ++meta_iter1) {
		if ((meta_iter1->tid != v2[0].tid))
			continue;
		isInclude = 1;
	}
	if (isInclude == 0) {
		for (std::vector<MetaInfo>::iterator meta_iter1 = v2.begin(); 
				meta_iter1 != v2.end(); ++meta_iter1) {
			v1.push_back((*meta_iter1));
		}
	}
	return isInclude;
}

int
JoinTree::find_virtual_col(JoinData data, utable_t tid, int col) {
	std::vector<MetaInfo> meta = data.meta;
	int i = 0;
	for (std::vector<MetaInfo>::iterator meta_iter = data.meta.begin(); 
			meta_iter != data.meta.end(); ++meta_iter) {
		if ((meta_iter->tid == tid) && (meta_iter->col == col)) {
			return i;
		}
		i++;
	}
	return -1;
}

void
JoinTree::setting_node(JoinNode* node, JoinInfo ji) {
	utable_t r_tid, s_tid;
	int r_col, s_col;
	bool isInclude;
	std::vector<MetaInfo> r_meta = node->inputR->output.meta;
	std::vector<MetaInfo> s_meta = node->inputS->meta;
	std::vector<MetaInfo> out_meta = r_meta;



	isInclude = sum_meta(out_meta, s_meta);
	node->output.meta = out_meta;

	r_tid = ji.input[0].first;
	s_tid = ji.input[1].first;
	r_col = ji.input[0].second;
	s_col = ji.input[1].second;



	node->virtual_col.first = find_virtual_col(node->inputR->output, r_tid, r_col);
	node->virtual_col.second = find_virtual_col(*(node->inputS), s_tid, s_col);

	node->virtual_key_position = node->inputR->virtual_key_position;
	
	if (isInclude == 0) {
		unumber_t sz = node->inputR->output.meta.size();
		node->virtual_key_position.push_back(sz);
	}
}


void 
JoinTree::make_tree(std::vector<JoinInfo> join_infos, std::vector<JoinData> join_datas) {
	JoinNode* r_node = NULL;
	JoinNode* out_node;
	JoinData* s_data;
	utable_t r_tid, s_tid;
	int r_col, s_col;

	for (std::vector<JoinInfo>::iterator infos_iter = join_infos.begin(); infos_iter != join_infos.end(); ++infos_iter) {
		r_tid = infos_iter->input[0].first;
		s_tid = infos_iter->input[1].first;
		r_col = infos_iter->input[0].second;
		s_col = infos_iter->input[1].second;

		if (r_node == NULL) {
			JoinData* r_data = make_data(join_datas, r_tid);
			out_node = make_leaf_node(r_data);
			this->join_point.push_back(out_node);
			r_node = out_node;
			//join_node_print(*out_node);
		}

		s_data = make_data(join_datas, s_tid);
		out_node = make_node(r_node, s_data);
		setting_node(out_node, *infos_iter);
		this->join_point.push_back(out_node);
		r_node = out_node;
		//join_node_print(*out_node);
	}
	this->header = out_node;
}

void
JoinTree::join(JoinNode* join_node) {
	int r_target_index = join_node->virtual_col.first;
	int s_target_index = join_node->virtual_col.second;
	std::vector<udata_t> op;

	//em-shi
	/*if(join_node->inputR != NULL) {
		if(join_node->inputR->inputR != NULL) {
			JoinNode* del = join_node->inputR->inputR;
			delete del->inputS;
			delete del;
			join_node->inputR->inputR = NULL;
		}
	}*/
	
	for (int i = 0; i < join_node->inputR->output.ops.size(); i++) {
		for (int j = 0; j < join_node->inputS->ops.size(); j++) {
			if (join_node->inputR->output.ops[i][r_target_index] == join_node->inputS->ops[j][s_target_index]) {
				op = join_node->inputR->output.ops[i];
				for (int k = 0; k < join_node->inputS->ops[j].size(); k++) 
					op.push_back(join_node->inputS->ops[j][k]);
				join_node->output.ops.push_back(op);
			}
		}
	}
	join_node->isDone = 1;
}

unumber_t 
JoinTree::join_all() {
	JoinNode* jn;	
	unumber_t sum = 0;
	for (int i = 1; i < this->join_point.size(); i++) {
		jn = this->join_point[i];
		join(jn);
		join_node_print(*jn);
	}
	
	for (int i = 0; i < header->output.ops.size(); i++) {
		for (int j = 0; j < header->virtual_key_position.size(); j++) {
			sum += header->output.ops[i][j];
		}
	}
	return sum;
}

void 
JoinTree::join_data_print(JoinData jd) {
	std::cout <<"meta: ";
	for (int i = 0; i <jd.meta.size(); i++) {
		std::cout << '(' << jd.meta[i].tid << ", " <<jd.meta[i].col << ") ";
	}
	std::cout << std::endl;
	std::cout <<"ops"<<std::endl;
	for (int i = 0; i < jd.ops.size(); i++) {
		for (int j = 0; j < jd.ops[i].size(); j++) {
			std::cout << jd.ops[i][j] << ' ';
		}
		std::cout << std::endl;
	}
}

void
JoinTree::join_node_print(JoinNode node) {
	if (node.isDone == 0)
		std::cout << "is done : False" << std::endl;
	else 
		std::cout << "is done : True" << std::endl;
	std::cout << "virtual col: " << node.virtual_col.first << ", " << node.virtual_col.second << std::endl;
	std::cout << "virtual key position: ";
	for (int i = 0; i < node.virtual_key_position.size(); i++) {
		std::cout << node.virtual_key_position[i] << ", ";
	
	}
	std::cout<<std::endl;
	if(node.inputS != NULL) 
		join_data_print(*node.inputS);
	std::cout << "----------------------------" <<std::endl;
	if(node.output.ops.size() != 0)
		join_data_print(node.output);
	std::cout << "============================" <<std::endl;
}
