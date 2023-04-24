
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream
#include <vector> 
#include <list> 
#include <thread> 
#include <random>
#include <memory>
#include <functional>
#include <queue>
#include <mutex>
#include <sstream>

using namespace std;

struct hiker
{
    string name;
    int speed;
};

/*
 * This is the class for calculating the result of the trip
 */
class result_of_hiking_trip
{
	vector<float> result_of_each;
	std::mutex m_result_lock;

public:
	result_of_hiking_trip()
	{
	}

	void store_in_result_array(float val)
	{
		result_of_each.push_back(val);
	}

	vector<float> get_result_array()
	{
		return result_of_each;
	}

	void result_lock()
	{
		m_result_lock.lock();
	}

	void result_unlock()
	{
		m_result_lock.unlock();
	}

	float calculate_result_array()
	{
		float total_resultant_time = 0;

		for(int i = 0; i < result_of_each.size(); i++)
		{
			total_resultant_time = result_of_each[i] + total_resultant_time;
		}
	
		return total_resultant_time;
	}
};

/*
 * Only one object is enough for the whole trip. 
 */
result_of_hiking_trip single_result_object;


int get_list_of_bridge_len(vector<float>& vec_blen)
{
    string next;
    ifstream fin;

	fin.open("hiking.yaml", ios::in);
	if(!fin)
	{
		cout<<"Error in opening the file"<<endl;
		return -1;
	}

	while(getline(fin,next))
	{
		int found = next.find("bridge");
		if(found != std::string::npos)
		{
			string sstr_blen = next.substr(next.find(":") + 1);
			std::string::size_type sz;   // alias of size_t
			int blen = stoi(sstr_blen, &sz);
			vec_blen.push_back(blen);
		}
                   
	}

	return 0;
}


/*
 * The functions of these class are executed at each Worker nodes.
 */
class calculate_fastest_time_each_bridge
{
	vector<struct hiker> hikers;
        int total_hikers;
	int bridge_length;
	int bridge_no;

public:

	calculate_fastest_time_each_bridge(vector<struct hiker> hkr, 
              int bridge_no, int total_hikers, float bridge_length)
	{
		for(int i = 0;i < hkr.size(); i++)
		{
                        struct hiker   h;
			h.name = hkr[i].name;
			h.speed = hkr[i].speed;
			this->hikers.push_back(h);
		}
		this->bridge_length = bridge_length;
		this->bridge_no = bridge_no;
		this->total_hikers = hkr.size();
	}

int get_no_of_hikers()
	{
		return total_hikers;
	}

	float get_length_of_bridge()
	{
		return bridge_length;
	}


int cal_time_to_cross_the_bridge()
{
    float fastest_time_to_cross_this_bridge = 0;
    struct hiker hkr; 
    hkr = this->hikers.back(); 
    float speed_of_torch_guy = hkr.speed;
    int no_of_hikers = get_no_of_hikers();
    int total_pair_trip = no_of_hikers - 1;
    int total_single_trip = total_pair_trip -1;
    float time_for_pair_trip = 0;
    float len_of_bridge = get_length_of_bridge();
    float time_for_single_trip = len_of_bridge / speed_of_torch_guy;
    float total_time_for_single_trip = time_for_single_trip * total_single_trip;
    float total_time_for_pair_trip = 0;
    this->hikers.pop_back();

    float speed ;
    for (auto&& r : this->hikers) {
        speed = r.speed;


	total_time_for_pair_trip = 
                 total_time_for_pair_trip + len_of_bridge / speed; 	
     } 

      fastest_time_to_cross_this_bridge = 
            total_time_for_single_trip + total_time_for_pair_trip; 

	return fastest_time_to_cross_this_bridge;
}
};

void thread_task(calculate_fastest_time_each_bridge& o) {
	vector<struct hiker> h;

	float each_time = o.cal_time_to_cross_the_bridge();
	ofstream result_file;


	result_file.open("result.out", ios::out | ios::app);
	if(!result_file)
	{
		cout<<"Error in opening the file result.out"<<endl;
	}
	else
	{
  		result_file << to_string(each_time);
  		result_file << " ";
  		result_file.close();
	}

	single_result_object.result_lock();
	single_result_object.store_in_result_array(each_time);
	single_result_object.result_unlock();
}

vector<struct hiker> create_list_for_job(list<struct hiker> *hlist, int job)
{
	list<struct hiker>::iterator it;
	vector<struct hiker> h;

	for(int i = 0;i <= job; i++)
	{
    		for (it = hlist[i].begin(); it != hlist[i].end(); ++it)
		{
			struct hiker hk;
			
			hk.name = it->name;
			hk.speed = it->speed;
			h.push_back(hk);

		}
	}

	return h;
}


// hlist
list<struct hiker> *hlist;

int main(int argc, char** argv) {
string next;
    ifstream fin;
   int job = 0;
	int nflag = 0;
	int sflag = 0;
	string n;
	float s;
	vector<float> vec_blen;
	int total_bridges = 0; 

     if (argc < 2 || argc > 2) {
	std::cerr << "Missing or extra parameters " << std::endl;
	return -1;
     }

     fin.open(argv[1], ios::in);

     if (!fin) {
	cout<<"Error in opening the file"<<endl;
		return -1;
     }

	if(get_list_of_bridge_len(vec_blen) != -1)
	{
		total_bridges = vec_blen.size();
	}
     
       hlist = new list<struct hiker>[total_bridges];
       int first  = 0; 

       while(getline(fin,next )) {
           int found = next.find("hikers");
   	   if(found != std::string::npos) {
   	       while(getline(fin,next)) {
                   int found1 = next.find("hikers");
	           int found2 = next.find("bridge");
	          if(found1 == std::string::npos && 
                        found2 == std::string::npos) {
	        	int f1 = next.find("name");
			if(f1 != string::npos) {
                            string sp = next.substr(next.find(":") + 1);
			    n = sp;
			    nflag = 1;
		        }
			int f2 = next.find("speed");
                        
			if(f2 != string::npos) {
			    string sp = next.substr(next.find(":") + 1);
			    std::string::size_type sz;   // alias of size_t
			    s = stof(sp,&sz);
		            sflag = 1;
			}
                               
			if(nflag && sflag) {
			    struct hiker hkr, hkr1;
			    hkr.speed = s;
                            if (!first) {
                               first++;   
 	 		        hlist[job].push_back(hkr);
                            } else {
                                hkr1 = hlist[job].back();
                                if (s > hkr1.speed) {
                                  hlist[job].push_back(hkr);
                                } else {
                                    hlist[job].pop_back();
                                    hlist[job].push_back(hkr);
                                    hlist[job].push_back(hkr1);
                                }

                            }
			    nflag = 0;
			    sflag = 0;
 			}
		} else {
		    job++;
		    break;
		}
	    }
        

	}
    }
    fin.close();

    int return1, total1;
    for(int i = 0; i <= job; i++) {

       vector<struct hiker> h;
       vector<float> vec_blen;

	h = create_list_for_job(hlist,i);


// Create threads for consurrency
	calculate_fastest_time_each_bridge obj(h, i, h.size(), vec_blen[i]);
        	std::thread t1(thread_task, std::ref(obj));
        	t1.join();
	}
	single_result_object.result_lock();
	float result = single_result_object.calculate_result_array();
	cout<<"Total fastest Time for the trip by using shared memory method = "<< " "<< result << " " <<"minutes"<< endl;
	single_result_object.result_unlock();

}





