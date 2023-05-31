#include "test.h"

using namespace std;

int test(
                   const char * hipoFile = "/cache/clas12/rg-a/production/montecarlo/clasdis/fall2018/torus-1/v1/bkg45nA_10604MeV/45nA_job_3051_0.hipo",
                   const char * rootfile = "output/file1.root"
)
{
    gROOT->ProcessLine("#include <vector>");
    //Below file is now disappeared...
//    auto hipoFile = "/cache/clas12/rg-a/production/montecarlo/clasdis/fall2018/torus-1/v1/bkg45nA_10604MeV/45nA_job_3301_3.hipo";
// Current files: defined in main function though
//    auto hipoFile = "/cache/clas12/rg-a/production/montecarlo/clasdis/fall2018/torus-1/v1/bkg45nA_10604MeV/45nA_job_3051_0.hipo";
//    auto rootFile = "OutputFiles/AffinityFiles/Files_9_5/Exactfile2.root";
    
    TFile *f = TFile::Open(rootfile,"RECREATE");
    
    HipoChain chain;
    
    //Add file to HipoChain
    chain.Add(hipoFile);
    auto config_c12 = chain.GetC12Reader();
    
    //Set PID cuts
    config_c12->addExactPid(11,1);    //exactly 1 electron
//    config_c12->addAtLeastPid(211,1);    //exactly 1 pi+
//    config_c12->addAtLeastPid(-211,1);    //exactly 1 pi-
    config_c12->addExactPid(2212,1);    //exactly 1 proton
    //Add MC::Lund bank for taking Lund data
    auto idx_MCLund= config_c12->addBank("MC::Lund");
    //Add a few items
    auto ipid=config_c12->getBankOrder(idx_MCLund,"pid"); 
    auto ipx=config_c12->getBankOrder(idx_MCLund,"px"); 
    auto ipy=config_c12->getBankOrder(idx_MCLund,"py"); 
    auto ipz=config_c12->getBankOrder(idx_MCLund,"pz"); 
    
    //Making new MC tree for dihadron
    TTree *tree_MC = new TTree("tree_MC","Tree with MC data from dihadron");
    tree_MC->Branch("P",&P);
    
    //This line comes from AnalysisWithExtraBanks.C
    auto& c12=chain.C12ref();
    int event_count = 0;
    while(chain.Next()==true){
        
        if(c12->getDetParticles().empty())
            continue;
        if(event_count % 200000 == 1)
            printf("Event #%d\n", event_count);
        if(event_count > 20)
            break;

        //Loop over MC::Lund entries in this event using its ID = idx_MCLund
        //Get PID from its id = iPid
        for(auto imc=0;imc<c12->getBank(idx_MCLund)->getRows();imc++){
            auto mcparticles = c12->mcparts();
            P = 0;
            px = 0;
            py = 0;
            pz = 0;
            pid = mcparticles->getPid(imc);
            parent = mcparticles->getParent(imc);
            if(pid != 11 || parent != 0) {
                continue;
            }
            px = mcparticles->getPx(imc);
            py = mcparticles->getPy(imc);
            pz = mcparticles->getPz(imc);
//             double P_calc = sqrt(px*px + py*py + pz*pz);
//             P = Pfunc(px, py, pz);
//             printf("P: %d; P_calc: %d\n", P, P_calc);
            P = 5;
            zbin0.zFillVectors(1,2,3,4,5,6);
        }
        tree_MC->Fill();
        event_count++;
    }
    f->Write();
    delete f;
    
    return 0;
}