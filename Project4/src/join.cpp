#include <iostream>
#include <string>
#include <vector>
#include "types.h"
#include "params.h"
#include "join.h"


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

		join_info.inputR.push_back(std::pair<utable_t, int>(tr, colr));
		
		join_info.inputS = std::pair<utable_t, int>(ts, cols);
		this->join_info.push_back(join_info);
	}
	
}

