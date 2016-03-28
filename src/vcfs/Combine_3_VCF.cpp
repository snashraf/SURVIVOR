/*
 * Combine_3_VCF.cpp
 *
 *  Created on: Mar 10, 2015
 *      Author: fsedlaze
 */

#include "Combine_3_VCF.h"
bool match_coords(strvcfentry c1, strvcfentry c2, int max_allowed_dist) {

	if ((strcmp(c1.start.chr.c_str(), c2.start.chr.c_str()) == 0
			&& abs(c1.start.pos - c2.start.pos) < max_allowed_dist)) {
		if(c1.type==4){
			return true;
		}
		return (strcmp(c1.stop.chr.c_str(), c2.stop.chr.c_str()) == 0
				&& abs(c1.stop.pos - c2.stop.pos) < max_allowed_dist);

	} else if ((strcmp(c1.stop.chr.c_str(), c2.start.chr.c_str()) == 0
			&& abs(c1.stop.pos - c2.start.pos) < max_allowed_dist)) {
		if(c1.type==4){
				return true;
			}
		return (strcmp(c1.start.chr.c_str(), c2.stop.chr.c_str()) == 0
				&& abs(c1.start.pos - c2.stop.pos) < max_allowed_dist);

	}
	return false;

}
int find_SV(strvcfentry caller,std::vector<strvcfentry> & merged,int max_dist ){

	for(size_t i =0;i<merged.size();i++){
		//std::cout<<caller.type<<" "<<merged[i].type<<" "<<caller.start.pos<<" "<<merged[i].start.pos<<std::endl;
		//std::cout<<caller.start.chr<<" "<<merged[i].start.chr<<" "<<caller.stop.pos<<" "<<merged[i].stop.pos<<std::endl;
		if(caller.type==merged[i].type && match_coords(caller, merged[i], max_dist)){
		//	std::cout<<"\t true"<<std::endl;
			return i;
		}
	}
	//std::cout<<"\t false"<<std::endl;
return -1;
}

void process_SV(std::vector<strvcfentry> caller,
		std::vector<strvcfentry> & merged, int max_dist) {
	std::vector<strvcfentry> new_merged = merged;

	for (size_t i = 0; i < caller.size(); i++) {
		int id = find_SV(caller[i], merged, max_dist);
		if (id == -1) {
			new_merged.push_back(caller[i]);
			new_merged[new_merged.size() - 1].sup_lumpy = 1;
		} else {
			//std::cout<<"Match"<<std::endl;
			new_merged[id].sup_lumpy++;
			merged[id].sup_lumpy++;
		}
	}
	//merged.clear();
	merged = new_merged;
}

void modify_entry(strvcfentry & entry){


	std::stringstream ss;
	size_t pos=entry.header.find_first_of(";");
	ss<<";SUP=";
	ss<<entry.sup_lumpy;
	entry.header.insert(pos,ss.str());
}

void combine_calls(std::string vcf_delly, std::string vcf_lumpy,
		std::string vcf_pindel, int max_dist, std::string output) {

	std::vector<strvcfentry> delly = parse_vcf(vcf_delly);
	std::vector<strvcfentry> lumpy = parse_vcf(vcf_lumpy);
	std::vector<strvcfentry> pindel = parse_vcf(vcf_pindel);

	std::vector<strvcfentry> merged;
	process_SV(pindel, merged, max_dist);
	std::cout<<"merged: "<<merged.size()<<std::endl;
	process_SV(delly, merged, max_dist);
	std::cout<<"merged: "<<merged.size()<<std::endl;
	process_SV(lumpy, merged, max_dist);
	std::cout<<"merged: "<<merged.size()<<std::endl;

	FILE * final;
	FILE * unique;
	std::string out = output;
	out += "_overlap.vcf";
	final = fopen(out.c_str(), "w");

	out = output;
	out += "_uniq.vcf";
	unique = fopen(out.c_str(), "w");

	print_header(vcf_delly, final);
	print_header(vcf_delly, unique);

	for (size_t i = 0; i < merged.size(); i++) {
		if (merged[i].sup_lumpy > 1) { //two callers must support the calls
			//modify_entry(merged[i]);
			print_entry(merged[i], final);
		} else {
			print_entry(merged[i], unique);
		}
	}
	fclose(final);
	fclose(unique);

}
