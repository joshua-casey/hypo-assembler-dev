/*
 * 
 * Copyright (c) 2019, Ritu Kundu and Joshua Casey
 *
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/** Defines the class Hypo.
 * It is the master class carrying out the polishing.
 */


#include <omp.h>

#include "Hypo.hpp"
#include "suk/SolidKmers.hpp"
#include "Window.hpp"



namespace hypo
{
    
Hypo::Hypo(const InputFlags& flags): _cFlags(flags), _monitor() {
    omp_set_num_threads(_cFlags.threads);
    _sk_done = _cFlags.done_stage>= STAGE_SK;
}
void Hypo::polish(const Mode mode, const FileNames& filenames) {
    _gStagefile.open(_cFlags.wdir+STAGEFILE, std::ofstream::out | std::ofstream::app);
    if (!_gStagefile.is_open()) {
        fprintf(stderr, "[Hypo::Hypo] Error: File open error: Stage File (%s) exists but could not be opened!\n",std::string(_cFlags.wdir+STAGEFILE).c_str());
        exit(1);
    }
    if (mode==Mode::SECOND){ // realignment done
        _monitor.start();
        std::string tm = _monitor.stop("[Hypo:Hypo]: Remapping reads done. ");
        _gStagefile << "Stage:Remapping [" << std::string(tm.c_str()) << "]\t" << STAGE_REMAP << std::endl;
    }
    std::cout << "............................ Starting Hypo (v"<<VERSION<< ") Polishing ............................\n";
    print_time();
    auto num_threads = _cFlags.threads;
    ///////////////////////////////////
    /* Get contigs */
    const bool is_initial = _cname_to_id.empty();
    _monitor.start();
    Contig::set_mode(mode);
    UINT32 cid = 0;
    gzFile fp = gzopen(filenames.draft_filename.c_str(), "r");
    kseq_t *seq;
    seq = kseq_init(fp);
    int l;
    while((l = kseq_read(seq)) >= 0) {
        if (is_initial) {
            _cname_to_id[std::string(seq->name.s,seq->name.l)] = cid;
        }
        else {
            cid = _cname_to_id[std::string(seq->name.s,seq->name.l)];
        }
        _contigs.emplace_back(std::make_unique<Contig>(cid, seq));
        ++cid;
    }
    _monitor.stop("[Hypo:Hypo]: Loaded Contigs. ");
    _contigs.shrink_to_fit();
    _salignment_store.resize(_contigs.size());
    _lalignment_store.resize(_contigs.size());
    ///////////////////////////////////
    /* Find solid positions in contigs */
    if (mode == Mode::LSA || mode == Mode::SECOND || mode == Mode::SO) {
        auto pSK = get_solid_kmers(filenames);
        _monitor.start();
        #pragma omp parallel for schedule(static,1) 
        for (UINT64 i = 0; i < _contigs.size(); ++i) {        
            _contigs[i]->find_solid_pos(pSK);
        }
        _monitor.stop("[Hypo:Hypo]: Found Solid pos in contigs. ");
    }

    ///////////////////////////////////
    /* Open Alignment files */
    // NOTE: Long aln in LSA should be uploaded before Short to handle Long INS (required before prepare for division)
    if (mode == Mode::SO || mode == Mode::LSA ||  mode == Mode::SECOND) {
        _sf_short = sam_open(filenames.sr_bam_filename.c_str(), "r");
        _sam_header_short = sam_hdr_read(_sf_short);
        _hts_align_short = bam_init1();
    }
    if (mode == Mode::LO || mode == Mode::LSA) {
        _sf_long = sam_open(filenames.lr_bam_filename.c_str(), "r");
        _sam_header_long = sam_hdr_read(_sf_long);
        _hts_align_long = bam_init1();
    }
    
    /** /////////////////////// BATCH WISE PROCESSING BEGINS //////////////////////////// **/
    _contig_batch_size = (_cFlags.processing_batch_size==0)?(_contigs.size()):(_cFlags.processing_batch_size);
    UINT32 num_batches = _contigs.size()/_contig_batch_size;
    if (_contigs.size()%_contig_batch_size!=0) {
        ++num_batches;
    }
    fprintf(stdout, "[Hypo::Hypo] Info: Number.of contigs: %lu; Number of batches: %u\n",_contigs.size(),num_batches);
    if (mode == Mode::LO || mode == Mode::LSA) {
        Window::prepare_for_poa(_cFlags.threads);
    }
    if (mode == Mode::SO || mode == Mode::LSA || mode == Mode::SECOND) {
        Window::prepare_for_cluster(_cFlags.threads);
    }
    
_monitor.start();
            create_alignments(false, mode, 0);
            _monitor.stop("[Hypo:Hypo]: Loaded alignments of Long reads. ");
exit(5);


    for (UINT32 batch_id = 0; batch_id< num_batches; ++batch_id) {
        fprintf(stdout, "********** [Hypo::Hypo] Info: BATCH-ID: %u\n",batch_id);
        print_time();
        UINT32 initial_cid = batch_id * _contig_batch_size;
        UINT32 final_cid = std::min(UINT32(_contigs.size()),initial_cid + _contig_batch_size);


        ///////////////////////////////////
        /* Find strong regions (SR) in contigs and divide WR into windows */
        if (mode == Mode::SO || mode == Mode::LSA || mode == Mode::SECOND) {
            _monitor.start();
            create_alignments(true, mode, batch_id);
            _monitor.stop("[Hypo:Hypo]: Loaded alignments. ");

            _monitor.start();
            for (UINT64 cid=initial_cid; cid < final_cid; ++cid) {
                if (cid==initial_cid || cid==final_cid-1 || cid%20==0) {
                    std::cout << "KSup: " << cid <<std::endl;
                }
                    
                #pragma omp parallel for
                for (UINT64 t = 0; t < _salignment_store[cid].size(); ++t) { 
                    _salignment_store[cid][t]->update_solidkmers_support(_cFlags.k, *(_contigs[cid]));
                }            
            }       
            _monitor.stop("[Hypo:Hypo]: Solid kmers support update. ");
        }
        /* Get long alignments (required before prepare_for division in LSA) */
        if (mode == Mode::LSA || mode == Mode::LO) { 
            _monitor.start();
            create_alignments(false, mode, batch_id);
            _monitor.stop("[Hypo:Hypo]: Loaded alignments of Long reads. ");
        }

        if (mode == Mode::SO || mode == Mode::LSA || mode == Mode::SECOND) {
            _monitor.start();
            #pragma omp parallel for schedule(static,1) 
            for (UINT64 i = initial_cid; i < final_cid; ++i) {
                // Divide into SR and MegaWindows; Prepare for minimser based cutting of MegaWindows
                // NOTE: Long aln in LSA should be uploaded before Short to handle Long INS (required before prepare for division)
                _contigs[i]->prepare_for_division(_cFlags.k, _cFlags.wdir);
            }
            UINT64 num_sr =0;
            UINT64 len_sr = 0;
            for (UINT64 i = initial_cid; i < final_cid; ++i) {
                num_sr += _contigs[i]->get_num_sr();
                len_sr += _contigs[i]->get_len_sr();
            }
            fprintf(stdout, "[Hypo::Hypo] Info: Total number of SR: %lu; Total length of SR: %lu\n",num_sr,len_sr); 
            _monitor.stop("[Hypo:Hypo]: Finding SR (and preparing for division into short windows). ");  
        }
        if (mode == Mode::SO || mode == Mode::SECOND) {
            _monitor.start();
            for (UINT64 cid=initial_cid; cid < final_cid; ++cid) {
                if (cid==initial_cid || cid==final_cid-1 || cid%20==0) {
                    std::cout << "MSup: " << cid <<std::endl;
                }
                #pragma omp parallel for
                for (UINT64 t = 0; t < _salignment_store[cid].size(); ++t) { 
                    _salignment_store[cid][t]->update_minimisers_support(*(_contigs[cid]));
                }            
            }  
            _monitor.stop("[Hypo:Hypo]: Minimisers support update. ");
        }
        _monitor.start();
        #pragma omp parallel for schedule(static,1) 
        for (UINT64 i = initial_cid; i < final_cid; ++i) {
            _contigs[i]->divide_into_regions();
        }
        _monitor.stop("[Hypo:Hypo]: Division of MegaWindows into short windows. ");
        ///////////////////////////////////
        /* Fill windows with short arms */
        if (mode == Mode::SO || mode == Mode::LSA || mode == Mode::SECOND) {
            _monitor.start();
            for (UINT64 cid=initial_cid; cid < final_cid; ++cid) {
                if (cid==initial_cid || cid==final_cid-1 || cid%20==0) {
                    std::cout << "SAln: " << cid <<std::endl;
                }
                #pragma omp parallel for
                for (UINT64 i = 0; i < _salignment_store[cid].size(); ++i) { 
                    _salignment_store[cid][i]->find_short_arms(_cFlags.k, mode, *(_contigs[cid]));
                }
            }
            _monitor.stop("[Hypo:Hypo]: Short arms computing. ");
            _monitor.start();
            // This will destroy alignments after use.
            #pragma omp parallel for schedule(dynamic,1) 
            for (UINT64 i = initial_cid; i < final_cid; ++i) {
                UINT64 initial_aid = 0;
                UINT64 final_aid = UINT64(_salignment_store[i].size());
                _contigs[i]->fill_short_windows(_salignment_store[i], initial_aid, final_aid);
                _salignment_store[i].clear();
            }
            _monitor.stop("[Hypo:Hypo]: Short arms filling. ");
        }
        ///////////////////////////////////
        /* Prepare Pseudo windows;Prepare invalid windows arms for long arms filling; */
        if (mode == Mode::LSA) { 
            #pragma omp parallel for schedule(static,1) 
            for (UINT64 i = initial_cid; i < final_cid; ++i) {
                _contigs[i]->prepare_pseudo_windows();
            }
        }
        ///////////////////////////////////
        /* Polish with short arms */
        if (mode == Mode::LSA || mode == Mode::SO || mode == Mode::SECOND) {   
            _monitor.start();
            for (UINT64 i = initial_cid; i < final_cid; ++i) {
                UINT64 num_short = _contigs[i]->get_num_swindows();
                #pragma omp parallel for schedule(static,1) 
                for (UINT64 w=0; w < num_short; ++w) {
                    auto tid = omp_get_thread_num();
                    _contigs[i]->generate_short_consensus(w,tid);   
                }
            }
            _monitor.stop("[Hypo:Hypo]: Polishing of Short windows. ");
        }
         ///////////////////////////////////
        /* Get long alignments */
        if (mode == Mode::LSA || mode == Mode::LO) { 
            ///////////////////////////////////
            /* Get long arms */
            _monitor.start();

            for (UINT64 cid=initial_cid; cid < final_cid; ++cid) {
                if (cid==initial_cid || cid==final_cid-1 || cid%20==0) {
                    std::cout << "LAln: " << cid <<std::endl;
                }
                #pragma omp parallel for
                for (UINT64 i = 0; i < _lalignment_store[cid].size(); ++i) { 
                    if (mode==Mode::LO) {
                        _lalignment_store[cid][i]->find_long_arms(*(_contigs[cid]));
                    }
                    else {
                        _lalignment_store[cid][i]->find_pseudo_arms(*(_contigs[cid]));
                    }
                }
            }
            _monitor.stop("[Hypo:Hypo]: Long arms computing. ");
            _monitor.start();            
            // This will destroy alignments after use. Also calls handling longDEL
            #pragma omp parallel for schedule(dynamic,1) 
            for (UINT64 i = initial_cid; i < final_cid; ++i) {
                UINT64 initial_aid = 0;
                UINT64 final_aid = UINT64(_lalignment_store[i].size());
                _contigs[i]->fill_long_windows(_lalignment_store[i], initial_aid, final_aid);
                _lalignment_store[i].clear();
            } 
            _monitor.stop("[Hypo:Hypo]: Long arms filling. ");
       
            /* Polish with long arms */        
            _monitor.start();
            for (UINT64 i = initial_cid; i < final_cid; ++i) {
                UINT64 num_long = _contigs[i]->get_num_lwindows();
                #pragma omp parallel for schedule(static,1) 
                for (UINT64 w=0; w < num_long; ++w) {
                    // Long window is always valid
                    auto tid = omp_get_thread_num();
                    _contigs[i]->generate_long_consensus(w,tid); 
                }
            }
            _monitor.stop("[Hypo:Hypo]: Polishing of Long windows. ");
        }

        // Overlap Long Ins handling
        if (mode == Mode::LSA) { 
            _monitor.start();
            #pragma omp parallel for schedule(dynamic,1) 
            for (UINT64 cid=initial_cid; cid < final_cid; ++cid) {
                _contigs[cid]->handle_ovl_li();
            }
            _monitor.stop("[Hypo:Hypo]: Ovl LI handled. ");
        }
    }
    
    /** /////////////////////// BATCH WISE PROCESSING ENDS //////////////////////////// **/
    _salignment_store.clear();
    _salignment_store.shrink_to_fit(); 
    _lalignment_store.clear();
    _lalignment_store.shrink_to_fit(); 

    ///////////////////////////////////
    /* Write results */
    _monitor.start();
    std::ofstream ofile(filenames.output_filename);
    if (!ofile.is_open()) {
        fprintf(stderr, "[Hypo::Hypo] Error: File open error: Output File (%s) could not be opened!\n",filenames.output_filename.c_str());
        exit(1);
    }
    std::ofstream bedfile(_cFlags.wdir+BEDFILE);
    for (UINT64 i = 0; i < _contigs.size(); ++i) {
       ofile << *(_contigs[i]);
       _contigs[i]->generate_inspect_file(_cFlags.wdir,bedfile);
    }
    ofile.close();
    _monitor.stop("[Hypo:Hypo]: Writing results. ");

    
    bedfile.close();
    _monitor.total("[Hypo:Hypo]: Overall. ");

    if (mode==Mode::LSA){ // First round of LSA done
        _monitor.start();
        std::string tm = _monitor.stop("[Hypo:Hypo]: LSA first round done. ");
        _gStagefile << "Stage:FirstRound [" << std::string(tm.c_str()) << "]\t" << STAGE_FIRST << std::endl;
    }

    // Clean-up
    _contigs.clear();
    _gStagefile.close();
}

std::unique_ptr<suk::SolidKmers> Hypo::get_solid_kmers(const FileNames& filenames){    
    auto num_threads = _cFlags.threads;    
    ///////////////////////////////////
    /* Get Solid kmers */
    std::unique_ptr<suk::SolidKmers> pSK;
    _monitor.start();
    
    if (!_sk_done) {
        pSK = std::make_unique<suk::SolidKmers>(_cFlags.k);
        bool is_success = pSK->initialise(filenames.sr_filenames,num_threads, _cFlags.sz_in_gb, SWindow_cov_settings.mean_cov, true, _cFlags.wdir+AUX_DIR);
        if (!is_success) {
            fprintf(stderr, "[Hypo::SolidKmers] Error: KMC Output: Could not have successful run of SUK for computing Solid kmers!\n");
            exit(1);
        }
        if (pSK->store(_cFlags.wdir+SKFILE)) {
            std::string tm = _monitor.stop("[Hypo:Hypo]: Computed Solid kmers. ");
            _gStagefile << "Stage:SolidKmers [" << std::string(tm.c_str()) << "]\t" << STAGE_SK << std::endl;
        }
        else {
            fprintf(stderr, "[Hypo::SolidKmers] Error: File Saving: Could not store the DS for Solid kmers!\n");
            exit(1);
        }
        _sk_done=true;
    }
    else {
        pSK = std::make_unique<suk::SolidKmers>(_cFlags.k);
        if (!(pSK->load(_cFlags.wdir+SKFILE))) {
            fprintf(stderr, "[Hypo::SolidKmers] Error: File Loading: Could not load the DS for Solid kmers (%s)!\n",std::string(_cFlags.wdir+SKFILE).c_str());
            exit(1);
        }
        _monitor.stop("[Hypo:Hypo]: Loaded Solid kmers. ");
    }
    fprintf(stdout, "[Hypo::Hypo] Info: Number of (canonical) solid kmers (nonhp) : %lu\n",pSK->get_num_solid_kmers()); 
    return pSK;
}



void Hypo::create_alignments(const bool is_sr, const Mode mode, const UINT32 batch_id) {
    UINT8 mq = _cFlags.map_qual_th;

    bool save_complete = is_sr && (mode==Mode::LSA);

    auto sf = (is_sr) ? (_sf_short) : (_sf_long);
    auto sam_header = (is_sr) ? (_sam_header_short) : (_sam_header_long);
    auto hts_align = (is_sr) ? (_hts_align_short) : (_hts_align_long);
    
    UINT32 initial_cid = batch_id * _contig_batch_size;
    UINT32 final_cid = initial_cid + _contig_batch_size;

    UINT64 num_invalid = 0;
    UINT64 num_alns = 0;
    UINT64 num_aln_read = 0;
    UINT64 progress= 0;
    while(sam_read1(sf, sam_header, hts_align)>=0) {
        if ((num_aln_read%100000000)==0) {
            std::cout << "Aln processed (in 100 M): " << progress <<std::endl;
            ++progress;
        }
        ++num_aln_read;
        // Ignore if unmapped/secondary/duplicate mapping or with failed QC
        if (hts_align->core.flag & (BAM_FUNMAP)) {
            if (save_complete) {add_read_to_realign(std::string(bam_get_qname(hts_align), hts_align->core.l_qname));} 
            continue;
        }
        if (hts_align->core.flag & (BAM_FSECONDARY | BAM_FQCFAIL | BAM_FDUP)) { 
            continue;
        }
        //if ((hts_align->core.flag & BAM_FPAIRED) && !(hts_align->core.flag & BAM_FPROPER_PAIR)) {continue;}
        // Ignore if mapping quality <=1 (only for short reads)
        if (is_sr && hts_align->core.qual < mq) {
            continue;
        }

        std::string cname(sam_hdr_tid2name(sam_header,hts_align->core.tid));
        if (_cname_to_id.find(cname) == _cname_to_id.end()) {
            fprintf(stderr, "[Hypo::Hypo] Error: Alignment File error: Contig-reference (%s) does not exist in the draft!\n",cname.c_str());
            exit(1);
        }
        UINT32 cid = _cname_to_id[cname];
        if (cid >= initial_cid) {
//if (hts_align->core.qual < mq){std::cout <<"Extra: cid:"<<cid<<" "<<cname<<" "<<hts_align->core.qual<<std::endl; }
            if (is_sr) {  // short reads     
                _salignment_store[cid].emplace_back(std::make_unique<Alignment>(*(_contigs[cid]), hts_align, save_complete));
                if (!((_salignment_store[cid].back())->is_valid)) {
                    _salignment_store[cid].pop_back();
                    --num_alns;
                    ++num_invalid;
                }
            }
            else { // long reads
                _lalignment_store[cid].emplace_back(std::make_unique<Alignment>(*(_contigs[cid]), hts_align, true, _cFlags.norm_edit_th));
                if (!((_lalignment_store[cid].back())->is_valid)) {
                    _lalignment_store[cid].pop_back();
                    --num_alns;
                    ++num_invalid;
                }
            }
            
            ++num_alns;
        }
        if (cid >= final_cid) { // cid belonging to next batch begins
            break;
        }            
    }
    fprintf(stdout, "[Hypo::Hypo] Info: Number of alignments (Batch %u): loaded (%lu) invalid (%lu)\n",batch_id,num_alns,num_invalid); 
    if (final_cid>=_contigs.size()) { // last batch done; clean up
        bam_destroy1(hts_align);
        sam_close(sf);
        
    } 
    if (!is_sr) {
        _monitor.start();
        for (UINT32 i=initial_cid; i < final_cid; ++i) {
            long_insert(i);
        }
        _monitor.stop("[Hypo:Hypo]: Long Insertion regions identified. ");
    } 

}


void Hypo::long_insert(UINT32 cid) {
    std::cout << "****************** LI " << cid << std::endl;
    // Create hash-tables for every 1M bp

    const UINT16 thres = std::ceil(0.3 * LWindow_cov_settings.mean_cov);
    const UINT16 mean_cov = LWindow_cov_settings.mean_cov;
    auto clen = _contigs[cid]->get_contig_len();
    auto num_ht = UINT32(clen >> HTSIZE);
    ++num_ht;
    std::cout << "Clen: " << clen <<" numht:"<< num_ht<< " mean:thres "<< mean_cov<<":"<<thres<<std::endl;
    std::vector<std::unordered_map<UINT32, UINT16>> bcnt(num_ht, std::unordered_map<UINT32, UINT16>()); // count for beg
    std::vector<std::unordered_map<UINT32, UINT16>> ecnt(num_ht, std::unordered_map<UINT32, UINT16>()); // count for end

    for (UINT32 i=0; i < _lalignment_store[cid].size(); ++i) {
        auto rb = _lalignment_store[cid][i]->get_rb();
        auto re = _lalignment_store[cid][i]->get_re();
	if (rb>0 && re <clen) {
        bcnt[(rb>>HTSIZE)][rb]++;
        ecnt[(re>>HTSIZE)][re]++;
	}
    }

    // Identify pos where exceptionally high reads end/start
    std::vector<UINT32> bp; //beg pos
    std::vector<UINT32> ep; //end pos
    for (UINT32 i=0; i < num_ht; ++i) {
        for (auto it=bcnt[i].begin(); it!=bcnt[i].end();++it) {
            if (it->second > thres) {
                bp.emplace_back(it->first);
                std::cout << "B "<<it->first << " " << it->second<< std::endl;
            }
        }
        
        for (auto it=ecnt[i].begin(); it!=ecnt[i].end();++it) {
            if (it->second > thres) {
                ep.emplace_back(it->first);
                std::cout << "E "<<it->first << " " << it->second<< std::endl;
            }
        }
    }
    std::stable_sort(bp.begin(), bp.end());
    std::stable_sort(ep.begin(), ep.end());

    long_indel(cid,bp,ep);
    gap_long_ins(cid,bp,ep);
}



void Hypo::long_indel (const UINT32 cid, const std::vector<UINT32>& bp, const std::vector<UINT32>& ep) {
    std::cout << "****************** OVL " << cid << std::endl;
    const UINT REP_UNIT = 1000;
    // pair bp and ep
    std::vector<UINT32> paired_bp; // bp of each pair
    std::vector<UINT32> paired_ep; // ep of each pair
    std::vector<UINT32> cov; // cov of each pair
    UINT32 pair_id = 0;


    UINT32 j = 0;
    for (UINT32 i=0; i < bp.size(); ++i) {
        if (j >= ep.size()) {break;}
        std::cout << "i:j " << i <<":"<<j<< " "<<bp[i] << " " << ep[j]<<std::endl;
        while (ep[j] < bp[i]) {++j;}
        if (i+1<bp.size() && bp[i+1] < ep[j]) {continue;} // next beg is closer to this end
        else {
            paired_bp.emplace_back(bp[i]);
            paired_ep.emplace_back(ep[j]);
            cov.emplace_back(0);
            std::cout << "Pushed " << pair_id<<std::endl;
            ++pair_id;
            ++j;
        }
    }

    if (paired_bp.empty()) {//std::cout << "NO OVL LI\n"; 
    return;}

    // At least one region here.
    // verify by read-sets; obtain pairs
    // Following works because aln are sorted
    std::vector<std::unordered_map<std::string, UINT32>> rn(pair_id, std::unordered_map<std::string, UINT32>()); // read-name to aln-id of first in pair;
    std::vector<std::vector<UINT32>> paired_faln(pair_id,std::vector<UINT32>()); // for each region, aln-index of first in pair
    std::vector<std::vector<UINT32>> paired_saln(pair_id, std::vector<UINT32>()); // for each region, aln-index of second in pair
    for (UINT32 i=0; i < _lalignment_store[cid].size(); ++i) {
        auto rb = _lalignment_store[cid][i]->get_rb();
        auto re = _lalignment_store[cid][i]->get_re();
        UINT32 qb=0;
        UINT32 qe=0;
        _lalignment_store[cid][i]->get_qcoords(qb,qe);
        auto qn = _lalignment_store[cid][i]->get_rname();
        auto searchb = std::lower_bound(paired_bp.begin(), paired_bp.end(), rb);
        auto lower = std::distance(paired_bp.begin(), searchb);
        auto searche = std::lower_bound(paired_ep.begin(), paired_ep.end(), re);
        auto upper = std::distance(paired_ep.begin(), searche);

        // Increase coverage 
        for (auto c=lower; c<upper; ++c) { ++cov[c];}
        if (searchb != paired_bp.end() && paired_bp[lower]==rb) { //read starts in one of the overlapping region
std::cout <<"= "<<i<<" l:"<<lower<<" u:"<<upper<<std::endl;
            auto prid = lower;
            auto pre = paired_ep[prid];
            std::cout << " Found b"<< rb << "-"<<re<<" "<<qn<<" q"<<qb<<"-"<<qe<<" in prid " << prid<<":"<<paired_bp[prid]<<"-"<<pre<<std::endl;
            auto searchrn = rn[prid].find(qn); 
            if (searchrn!=rn[prid].end() && re>pre) { // the read also ends at the overlapping region
                paired_faln[prid].emplace_back(searchrn->second);
                paired_saln[prid].emplace_back(i);
                std::cout << "Pairing "<<searchrn->first<< " "<<i <<" and "<<searchrn->second<<std::endl;
            }
        }

        if (searche != paired_ep.end() && paired_ep[upper]==re) { //read ends in one of the overlapping region
std::cout <<"= "<<i<<" l:"<<lower<<" u:"<<upper<<std::endl;
            auto prid = upper;
            auto prb = paired_bp[prid];
            std::cout << " Found e"<< rb << "-"<<re<<" "<<qn<<" q"<<qb<<"-"<<qe<<" in prid " << prid<<":"<<paired_bp[prid]<<"-"<<paired_ep[prid]<<std::endl;
            if (rb<prb) {
                rn[prid][qn] = i;
            }
        }
    }

    std::vector<UINT32> li_ind;
    for (UINT32 i=0; i < pair_id; ++i) {
        auto b = paired_bp[i];
        auto e = paired_ep[i];
        UINT32 suppth = std::ceil(LI_READ_PERC1 * cov[i]);
        std::cout << "Verifying "<<i << " cov:"<<cov[i]<<" supth:"<<suppth <<" sup:"<< paired_faln[i].size() <<std::endl;
        if (paired_faln[i].size() > 0 && paired_faln[i].size() >= suppth) {
            li_ind.emplace_back(i);
            std::cout << "Done1 " <<i <<"\t"<<cid<< "\t"<<paired_bp[i]<<"\t"<<paired_ep[i]<<std::endl;
        }
    }

    // Get the repeat size; get it from the first read in a region, verify with others
    for (auto p: li_ind) {
        // Get size from first pair.,;
        bool should_add = true;
        INT64 rep_len = 0; 
        INT64 del_len = 0;
        char indel_type = 'I';
        for (UINT32 i=0; i < paired_faln[p].size(); ++i) {
            auto f = paired_faln[p][i];
            auto s = paired_saln[p][i];
            UINT32 fqb = 0;
            UINT32 fqe = 0;
            UINT32 sqb = 0;
            UINT32 sqe = 0;
            _lalignment_store[cid][f]->get_qcoords(fqb,fqe);
            _lalignment_store[cid][s]->get_qcoords(sqb,sqe);
            bool frev = _lalignment_store[cid][f]->is_rev();
            bool srev = _lalignment_store[cid][s]->is_rev();
            INT64 diff = 0;
            if (!frev && !srev) {diff=(INT64)fqe-(INT64)sqb;}
            else if (frev && srev) {diff=(INT64)sqe-(INT64)fqb;}
            else {
                fprintf(stdout, "[Hypo::Hypo] Warning: Ovl Long Ins: Reads overlaps have discrepansies. (rep: %d; diff: %d)\n",rep_len,diff); 
                should_add = false;
                break;
            }
            std::cout <<"f:s"<<f<<":"<<s<<" fqb:fqe "<<fqb<<":"<<fqe<<"; sqb:sqe "<<sqb<<":"<<sqe<<"; frev:srev "<<frev<<":"<<srev<<" diff:"<<diff<<std::endl;
            if (i==0) {
                if (diff<0) {indel_type='D';del_len=std::abs(diff);}
                rep_len = ((std::abs(diff)/REP_UNIT)+1)*REP_UNIT;
            }
            else {// verify
                if (std::abs(diff)>rep_len || (indel_type=='I' && diff<0) || (indel_type=='D' && diff>0)) {
                    fprintf(stdout, "[Hypo::Hypo] Warning: Ovl Long Ins: Reads overlaps have discrepansies. (rep: %d; diff: %d)\n",rep_len,diff); 
                    should_add = false;
                    break;
                }               
            }
        }
        if (should_add) {
            std::cout << "Indel type: " << indel_type <<" "<<paired_bp[p]<<" replen:"<<rep_len<<" dellen:"<<del_len<<std::endl;
            if (indel_type=='I') { //long ins
                _contigs[cid]->add_ovl(paired_bp[p],rep_len);
            }  
            else { // long del
                _contigs[cid]->add_ld(paired_bp[p],del_len);
            }          
        }
    }
}


void Hypo::gap_long_ins (const UINT32 cid, const std::vector<UINT32>& bp, const std::vector<UINT32>& ep)
{
    std::cout << "****************** GAP " << cid << std::endl;
    const UINT GAP_DIFF = 5;
     const UINT HI_GAP_DIFF = 200;
	const UINT GAP_LEN_TH = std::ceil(0.5*Window_settings.ideal_lwind_size);
const UINT DEL_UNIT = 1000;
    // pair bp and ep
    std::vector<UINT32> paired_bp;
    std::vector<UINT32> paired_ep;
    std::vector<UINT32> cov; // cov of each pair
    UINT32 pair_id = 0;

    UINT32 j = 0;
    for (UINT32 i=0; i < ep.size(); ++i) {
        if (j >= bp.size()) {break;}
        std::cout << "i:j " << i <<":"<<j<< " "<<ep[i] << " " << bp[j]<<std::endl;
        while (bp[j] <= ep[i]) {++j;}
        if (i+1<ep.size() && ep[i+1] < bp[j]) {continue;} // next end is closer to this beg
        else {
	    //std::cout << "Insufficient Gap len\n";
            //if (bp[j]-ep[i] >= GAP_LEN_TH) {
            paired_bp.emplace_back(bp[j]);
            paired_ep.emplace_back(ep[i]);
            cov.emplace_back(0);
            std::cout << "Pushed " << pair_id<<std::endl;
            ++pair_id;
            ++j;
	   //}
//else{std::cout << "Insufficient Gap len d:th "<<bp[j]-ep[i]<<" "<<GAP_LEN_TH<<std::endl;}
        }
    }

    if (paired_bp.empty()) {//std::cout << "NO GAP LI\n"; 
    return;}

    // At least one region here.
    // verify by read-sets; obtain pairs
    // Following works because aln are sorted
    std::vector<std::unordered_map<std::string, UINT32>> rn(pair_id, std::unordered_map<std::string, UINT32>()); // read-name to aln-id of first in pair;
    std::vector<UINT32> paired_sup(pair_id,0); // for each region, num read-pairs supoorting it
	std::vector<INT32> paired_diff(pair_id,-1); // for each region, length of missing part; first read fills it, rest verify


    for (UINT32 i=0; i < _lalignment_store[cid].size(); ++i) {
        auto rb = _lalignment_store[cid][i]->get_rb();
        auto re = _lalignment_store[cid][i]->get_re();
        UINT32 qb=0;
        UINT32 qe=0;
        _lalignment_store[cid][i]->get_qcoords(qb,qe);
        auto qn = _lalignment_store[cid][i]->get_rname();
        auto searchb = std::lower_bound(paired_bp.begin(), paired_bp.end(), rb);
        auto lower = std::distance(paired_bp.begin(), searchb);
        auto searche = std::lower_bound(paired_ep.begin(), paired_ep.end(), re);
        auto upper = std::distance(paired_ep.begin(), searche);
       // Increase coverage 
        for (auto c=lower; c<upper; ++c) { ++cov[c];}
        if (searchb != paired_bp.end() && paired_bp[lower]==rb) { //read starts in one of the overlapping region
            auto prid = lower;
            auto pre = paired_ep[prid];
std::cout <<"* "<<i<<" l:"<<lower<<" u:"<<upper<<std::endl;
            std::cout << " Found b"<< rb << "-"<<re<<" "<<qn<<" q"<<qb<<"-"<<qe<<" in prid " << prid<<":"<<paired_bp[prid]<<"-"<<pre<<std::endl;
            auto searchrn = rn[prid].find(qn); 
            if (searchrn!=rn[prid].end()) { // the read also ends at the gap region;
                std::cout << "Pairing "<<searchrn->first<< " "<<i <<" and "<<searchrn->second<<std::endl;
                auto f = searchrn->second;
                UINT32 fqb = 0;
                UINT32 fqe = 0;
                _lalignment_store[cid][f]->get_qcoords(fqb,fqe);
                bool frev = _lalignment_store[cid][f]->is_rev();
                bool srev = _lalignment_store[cid][i]->is_rev();
                INT64 diff = 0;
		bool same_orn=false;
                if (!frev && !srev) {same_orn=true;diff=(INT64)qb-(INT64)fqe;}
                else if (frev && srev) {same_orn=true;diff=(INT64)qe-(INT64)fqb;}
                std::cout <<"fqb:fqe"<<fqb<<":"<<fqe<<"; sqb:sqe "<<qb<<":"<<qe<<"; frev:srev "<<frev<<":"<<srev<<" diff:"<<diff<<std::endl;
                if (same_orn && std::abs(diff)<=GAP_DIFF) {
                    ++paired_sup[prid];
                    std::cout<<"Supported "<<std::endl;
                }
		else if (same_orn && std::abs(diff)>=HI_GAP_DIFF) {
                    if (paired_diff[prid]==-1) { // not set yet
                        paired_diff[prid] = std::abs(diff);
                        ++paired_sup[prid];
std::cout << "Added. DSupported\n";
                    }
                    else {// verify
                        if ((((paired_diff[prid]/DEL_UNIT)*DEL_UNIT)+DEL_UNIT)>=(std::abs(diff))) {
                            fprintf(stdout, "[Hypo::Hypo] Warning: GAP Long DEL: Reads overlaps have discrepansies. (: %d; diff: %d)\n",paired_diff[prid],diff); 
                            ++paired_sup[prid];
			std::cout << "DSupported\n";
                        }               
                    }                    
                }
            }
        }
        if (searche != paired_ep.end() && paired_ep[upper]==re) { //read ends in one of the gap region
std::cout <<"= "<<i<<" l:"<<lower<<" u:"<<upper<<std::endl;
            auto prid = upper;
            auto prb = paired_bp[prid];
            std::cout << " Found e"<< rb << "-"<<re<<" "<<qn<<" q"<<qb<<"-"<<qe<<" in prid " << prid<<":"<<paired_bp[prid]<<"-"<<paired_ep[prid]<<std::endl;
            rn[prid][qn] = i;
        }
    }

    for (UINT32 i=0; i < pair_id; ++i) {
        auto b = paired_bp[i];
        auto e = paired_ep[i];
        //UINT32 supp = std::ceil(LI_READ_PERC2 * cov[i]);
        std::cout << "Verifying "<<i << " "<<std::ceil(LI_READ_PERC2 * cov[i])<<" "<< std::ceil(LI_READ_PERC3 * cov[i]) <<" "<< paired_sup[i] <<std::endl;
        if (paired_diff[i]==-1) { // Gapped INS
            UINT32 supp = std::ceil(LI_READ_PERC2 * cov[i]);
            if (paired_sup[i] > 0 && paired_sup[i] >= supp) {
                _contigs[cid]->add_gap(paired_ep[i],paired_bp[i]);
                std::cout << "Done2 " <<i <<"\t"<<cid<< "\t"<<paired_bp[i]<<"\t"<<paired_ep[i]<<std::endl;
            }
        }
        else { // LONG DEL
            UINT32 supp = std::ceil(LI_READ_PERC3 * cov[i]);
            if (paired_sup[i] > 0 && paired_sup[i] >= supp) {
                _contigs[cid]->add_ld(paired_ep[i],paired_diff[i]);
                std::cout << "Done4 " <<i <<"\t"<<cid<< "\t"<<paired_bp[i]<<"\t"<<paired_ep[i]<<std::endl;
            }
        }   
    }
}


} // namespace hypo

