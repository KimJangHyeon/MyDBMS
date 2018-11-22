typdef struct _ColInfo {
	int index;
	udata_t min;
	udata_t max;
}

typedef struct _TableInfo {
	unumber_t num_key;
	utable_t tid;
	vector<ColInfo> col;
} TableInfo;


