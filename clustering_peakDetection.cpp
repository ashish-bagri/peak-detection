// clustering_peakDetection.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <string>
#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <list>
#include <vector>
#include <numeric>
using namespace std;
void doclustering(list<float>& energyvalues,vector<float>& silence_confidence,float thresholdRatio, float windowsize,string clusterfile);
void usage()
{
	cout<<"clustering_peakDetection.exe inputfile threshold_wrt_mean windowSize"; 
}

int main(int argc, char** argv)
{
	if(argc <4)
	{
		usage();
		exit(0);
	}
	float thresholdRatio  = atof(argv[2]);
	float windowSize = atof(argv[3]);

	cout<<"Threshold ratio entered by you: "<<thresholdRatio<<endl;
	cout<<"Window size selected is : "<<windowSize<<endl;

	string inputFileName = argv[1];	
	ifstream input(argv[1],ios::in);
	if(!input.good())
	{
		cout<<"cannot open file "<<inputFileName<<endl;
		return 1;
	}

	list<float> inputvalues;
	ofstream output;	
	string outputFileName ;
	string clusterFileName;
	int lastindx = inputFileName.find_last_of(".");
	outputFileName = inputFileName.substr(0,lastindx);
	clusterFileName = inputFileName.substr(0,lastindx);
	outputFileName.append("_out");		
	clusterFileName.append("_cluster");
	outputFileName.append(inputFileName.substr(lastindx,inputFileName.length() - lastindx));
	clusterFileName.append(inputFileName.substr(lastindx,inputFileName.length() - lastindx));
	output.open(outputFileName,ios::out);


	std::copy(  std::istream_iterator<float>(input),
		std::istream_iterator<float>(),
		std::back_inserter(inputvalues));

	vector<float> silence_confidence(inputvalues.size());

	doclustering(inputvalues,silence_confidence,thresholdRatio,windowSize,clusterFileName);

	for(int i=0;i<silence_confidence.size();i++)
	{
		output<<silence_confidence[i]<<endl;
	}
	output.close();		
	return 0;
}

void doclustering(list<float>& energyvalues,vector<float>& silence_confidence,float thresholdRatio, float windowsize,string clusterfile)
{
	ofstream cluster(clusterfile,ios::out);
	int clustercount = 0;
	int MIN_CLUSTER_COUNT = 0;

	float mean = accumulate(energyvalues.begin(),energyvalues.end(),0.0) / energyvalues.size();
	int range = windowsize;

//	do{
//		clustercount = 0;
		float thresholdUsed = thresholdRatio*mean;
		list<float>::iterator it = energyvalues.begin();
		list<float>::iterator it2 = energyvalues.begin();
		list<float>::iterator it3 = energyvalues.begin();
		vector<float>::iterator itout = silence_confidence.begin();


		unsigned int c=0;
		float max,max_before,max_after;


		for(;it!=energyvalues.end();it++)
		{
			++c;
			if(c < range || c > energyvalues.size() - range)
			{
				max = *it;	
				max_before = *it;
				max_after = *it;
				if(*it >= thresholdUsed)
					*itout++ = 1;
				else
					*itout++ = 0;
				continue;
			}
			else{
				it2 = it;
				it3 = it;
				advance(it2,range);
				advance(it3,-range);

				max_before = *max_element(it3,it);
				max_after = *max_element(it,it2);  
			}
			if(max_before > thresholdUsed && max_after > thresholdUsed)// || max_before > mean/2.0f && max_after < mean/2.0f)
			{
				//sure case
				*itout++ = 1;
			}else 
			{
				*itout++ = 0;
			}	
		}

		vector<float>::iterator itsil = silence_confidence.begin();
		vector<float>::iterator itn2p;
		float temp_max = 0;
		int temp_pos = 0;
		it = energyvalues.begin();
		float prev = *itsil;
		for(;itsil!=silence_confidence.end();itsil++,it++)
		{
			if(prev == 0 && *itsil == 1)
			{
				//from negative to positive 
				// note the start of the pointer
				itn2p = itsil;
			}

			if(*itsil == 1)
			{
				prev = *itsil;
				if(*it > temp_max)
				{
					temp_max = *it;
					temp_pos = itsil - silence_confidence.begin();
				}
				continue;
			}

			// if *itsil == 0 !
			if(*itsil == 0 && prev == 1)
			{
				// from positive to negative.. need to check if the previous positive batch was OK! -- i.e the max was in the range
				if(temp_max > thresholdUsed)
				{
					clustercount++;
					// good ! so the previous positive batch had a max in it.. just move forward
					// here put down the details			
					//				cluster<<"Cluster Number "<<++clustercount<<endl;
					//				cluster<<"start point of cluster:: "<<itn2p - silence_confidence.begin() + 1<<endl;
					//				cluster<<"end point of cluster:: "<<itsil - silence_confidence.begin() <<endl;
					//				cluster<<"max value in the cluster:: "<<temp_max <<endl;
					//				cluster<<"position of the max value in the cluster:: "<<temp_pos + 1<<endl;
				}else
				{
					// the previous batch did not have any max in it.. so it was a false positive.. make it negative
					for(;itn2p!=itsil;itn2p++)
					{
						*itn2p  = 0;
					}
				}	
				temp_max = 0;
				temp_pos = 0;
			}
			prev = *itsil;		
		}	
		cout<<"Mean "<<mean<<endl;
		cout<<"Threshold used "<<thresholdUsed<<endl;
//		cout<<"cluster count "<<clustercount<<endl;
//		thresholdRatio  = 0.9*thresholdRatio;
//	}while(clustercount < MIN_CLUSTER_COUNT);
}