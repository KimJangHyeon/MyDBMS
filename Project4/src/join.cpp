#include <iostream>
#include <algorithm>
#include <iterator>
#include <string>
#include <vector>
#include <memory>

#include "types.h"
#include "params.h"
#include "join_struct.h"
#include "views.h"
#include "join.h"


void 
JoinSet::put_join_infos(std::string query) {
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
		for (int i = 0; i < 2; i++)
			join_info.input[i] = std::pair<utable_t, int>(t[i], col[i]);
	}

}

void 
JoinSet::parser(std::string query) {
	put_join_infos(query);
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
