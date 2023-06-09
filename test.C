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
    
    int hash_count = 0;
         
    //Making new MC tree for dihadron
    TTree *tree_MC = new TTree("tree_MC","Tree with MC data from dihadron");
    tree_MC->Branch("z",&z_h);
    tree_MC->Branch("z_N",&z_N);
    tree_MC->Branch("x",&x);
    tree_MC->Branch("pT",&pt_gN);
    tree_MC->Branch("Q2",&Q2);
    tree_MC->Branch("Q2calc",&Q2_calc);
    tree_MC->Branch("R0",&R0); //initial parton momentum
    tree_MC->Branch("R1",&R1); //Measured in gN frame
    tree_MC->Branch("R2",&R2);
    tree_MC->Branch("Mh",&Mdihadron);
    tree_MC->Branch("q_TdivQ",&qTQ);
    tree_MC->Branch("xF",&xF);
    tree_MC->Branch("qTQ_lab",&qTQ_lab);
    tree_MC->Branch("qTQfrac",&qTQfrac);
    tree_MC->Branch("qTQ_hadron",&qTQ_hadron);
    
    //Tell the user that the loop is starting
    cout << "Start Event Loop" << endl;
    int tree_count = 0;
    //now get reference to (unique)ptr for accessing data in loop
    //this will point to the correct place when file changes
    //
    //This line comes from AnalysisWithExtraBanks.C
    auto& c12=chain.C12ref();
    int event_count = 0;
    
    //Loop over all events in the file
    while(chain.Next()==true){
        
        event_count += 1;
        if(event_count == 1) {
            cout << '\n';
            cout << "\033[96m";
            cout << "\t\t\t\t" << " ~~~~~~~~~~~~" << '\n';
            cout << "\t\t\t\t" << "|Progress Bar|" << '\n';
            cout << "\t\t\t\t" << " ~~~~~~~~~~~~" << '\n';
            cout << "\t\t[";
        }
        if(event_count % 16388 == 0) {
            
            hash_count += 1;
//            cout << "\033[A" << "\033[A";
            cout << '\r' << "\t[";
            for (int i = 1; i < hash_count + 1;i++) {
                cout << '#';
            }
            for (int i = 0; i < 100 - hash_count; i++) {
                cout << ' ';
            }
            cout << ']' << ' ';
            cout << event_count / 16388. << '%';
            if(event_count / 16388. == 100) {
                cout << endl;
            }
            cout << flush;
        }
        //Break at event 100 for testing with shorter run time
/*        if(event_count >= 1000) {
            cout << "Breaking at event: " << event_count << '\n';
            break;
        }*/
        if(c12->getDetParticles().empty())
            continue;
        
        //Intializing MCParticles
        MCParticle electron;
        MCParticle proton;
        MCParticle photon;
        MCParticle Lund;

        Pidi piplus;
        Pidi piminus;

        Quark quark;

        Pidi diquark;
        
        MultiParticle Hadron;
        //Loop over MC::Lund entries in this event using its ID = idx_MCLund
        //Get PID from its id = iPid
        for(auto imc=0;imc<c12->getBank(idx_MCLund)->getRows();imc++){
            auto mcparticles = c12->mcparts();
            
            id = mcparticles->getIndex(imc);
            pid = mcparticles->getPid(imc);
            px = mcparticles->getPx(imc);
            py = mcparticles->getPy(imc);
            pz = mcparticles->getPz(imc);
            daughter = mcparticles->getDaughter(imc);
            parent = mcparticles->getParent(imc);
            mass = mcparticles->getMass(imc);
            P = Pfunc(px,py,pz);
            E = Efunc(mass,P);
            vz = mcparticles->getVz(imc);
            //
            //Kinematics
            // 


            //Setting scattered electron
            if(pid==11 && parent==1){
                electron.fillParticle(id, pid, px, py, pz, daughter, parent, mass, vz);
                electron.setVectors();
            }
            //pi+
            else if(pid==pipluspid){
                piplus.fillParticle(id, pid, px, py, pz, daughter, parent, mass, vz);
                piplus.update(id, pid, px, py, pz, daughter, parent, 
                              mass, vz);
            }
            //pi-
            else if(pid==piminuspid){
                piminus.fillParticle(id, pid, px, py, pz, daughter, parent, mass, vz);
                piminus.update(id, pid, px, py, pz, daughter, parent, 
                              mass, vz);
            }
            //all quarks
            else if(std::count(vquarklist.begin(), vquarklist.end(), pid)){
                quark.fillParticle(id, pid, px, py, pz, daughter, parent, mass, vz);
                quark.update(id, pid, px, py, pz, daughter, parent, 
                              mass, vz);
            }
            //MCParticle
            else if(pid==92 || pid == 91){
                Lund.fillParticle(id, pid, px, py, pz, daughter, parent, mass, vz);
                Lund.setVectors();
            }
            
            else if(std::count(vdiquarklist.begin(), vdiquarklist.end(), pid)){
                diquark.fillParticle(id, pid, px, py, pz, daughter, parent, mass, vz);
                diquark.update(id, pid, px, py, pz, daughter, parent, 
                              mass, vz);
            }
            else if(pid == 22 && parent == 1){
                photon.fillParticle(id, pid, px, py, pz, daughter, parent, mass, vz);
                photon.setVectors();
            }
            else if(id == 2){
                proton.fillParticle(id, pid, px, py, pz, daughter, parent, mass, vz);
                proton.setVectors();
            }
            else if(std::count(vhadronlist.begin(), vhadronlist.end(), pid)) {
                Hadron.fillParticle(id, pid, px, py, pz, daughter, parent, mass, vz);
                Hadron.update(id, pid, px, py, pz, daughter, parent, 
                              mass, vz);
            }
        }
        
        //Selecting pions that come from Lund particle
        for(int i = 0; i < piplus.v_id.size(); i++) {
            if(piplus.v_parent[i] == Lund.id) {
                piplus.select_id = i;
            }
        }
        for(int i = 0; i < piminus.v_id.size(); i++) {
            if(piminus.v_parent[i] == Lund.id) {
                piminus.select_id = i;
            }
        }
        
        //Selecting initial quark
        for(int i = 0; i < quark.v_id.size(); i++) {
            if(quark.v_parent[i] == 0) {
                quark.final_id = i;
            }
        }
        
        //Selecting diquark
        for(int i = 0; i < diquark.v_id.size(); i++) {
            if(diquark.v_parent[i] == 2) {
                diquark.select_id = i;
            }
        }
        
        //Skip non-dipion events
        if(piplus.select_id == -999) {
            continue;
        }
        else if(piminus.select_id == -999) {
            continue;
        }
        if(diquark.select_id != -999) {
            diquark.fillParticle(diquark.v_id[diquark.select_id], diquark.v_pid[diquark.select_id], diquark.v_px[diquark.select_id], diquark.v_py[diquark.select_id], 
                           diquark.v_pz[diquark.select_id], diquark.v_daughter[diquark.select_id], diquark.v_parent[diquark.select_id], diquark.v_mass[diquark.select_id], diquark.v_vz[diquark.select_id]);
            diquark.setVectors();
        }
        piplus.fillParticle(piplus.v_id[piplus.select_id], piplus.v_pid[piplus.select_id], piplus.v_px[piplus.select_id], piplus.v_py[piplus.select_id], 
                           piplus.v_pz[piplus.select_id], piplus.v_daughter[piplus.select_id], piplus.v_parent[piplus.select_id], piplus.v_mass[piplus.select_id], piplus.v_vz[piplus.select_id]);
        piplus.setVectors();
        piminus.fillParticle(piminus.v_id[piminus.select_id], piminus.v_pid[piminus.select_id], piminus.v_px[piminus.select_id], piminus.v_py[piminus.select_id], 
                           piminus.v_pz[piminus.select_id], piminus.v_daughter[piminus.select_id], piminus.v_parent[piminus.select_id], piminus.v_mass[piminus.select_id], piminus.v_vz[piminus.select_id]);
        piminus.setVectors();
        
        quark.fillParticle(quark.v_id[quark.final_id], quark.v_pid[quark.final_id], quark.v_px[quark.final_id], quark.v_py[quark.final_id], 
                           quark.v_pz[quark.final_id], quark.v_daughter[quark.final_id], quark.v_parent[quark.final_id], quark.v_mass[quark.final_id], quark.v_vz[quark.final_id]);
        quark.setVectors();
       
        //Setting inital beam and target particles
        init_electron.SetPxPyPzE(0, 0, sqrt(electron_beam_energy * electron_beam_energy - electronMass * electronMass), electron_beam_energy);
        init_target.SetPxPyPzE(0, 0, 0, proton.E);
        
        
        dihadron = piplus.lv + piminus.lv;
        m_plus = piplus.mass;
        m_minus = piminus.mass;
        Mdihadron = dihadron.M();
        Pdihadron = dihadron.P();
//         q = init_electron - electron.lv; //virtual photon
        q = photon.lv;
        q_calc = init_electron - electron.lv;
        qx = q_calc.Px();
        qy = q_calc.Py();
        qz = q_calc.Pz();
        qE = q_calc.E();
        
        photonx = photon.lv.Px();
        photony = photon.lv.Py();
        photonz = photon.lv.Pz();
        photonE = photon.lv.E();
        
        
        //Missing mass
        Mx = Mxfunc(q, init_target, piplus.lv, piminus.lv);

        cth = cthfunc(electron.px,electron.py,electron.pz);
        Q2 = -(q * q);
        Q2_calc = Q2func(electron_beam_energy,electron.E,cth); //Momentum transfer

        z_h_plus = (init_target * piplus.lv) / (init_target * q);
        z_h_minus = (init_target * piminus.lv) / (init_target * q);
        z_h = z_h_plus + z_h_minus;
        s = sfunc(protonMass, electronMass, electron_beam_energy);
        y = yfunc(electron_beam_energy,electron.E);
        x = Q2/s/y; // Bjorken x
        pt_lab = Ptfunc(dihadron.Px(), dihadron.Py()); //hadron transverse momentum
        
        kf = quark.lv;
        ki = kf - q;
        //Cut Kinematics
        
        gN = q;
        gN += init_target;
        gNBoost = gN.BoostVector();
        gNBoostNeg = -gNBoost;
        
        lv_p1_gN = piplus.lv;
        lv_p2_gN = piminus.lv;
        lv_p1_gN.Boost(gNBoostNeg);
        lv_p2_gN.Boost(gNBoostNeg);
        
        lv_q_gN = q;
        lv_q_gN.Boost(gNBoostNeg);
        
        q_gNx = lv_q_gN.Px();
        q_gNy = lv_q_gN.Py();
        q_gNz = lv_q_gN.Pz();
        q_gNE = lv_q_gN.E();
        
        //Need dihadron in gN frame for pT
        dihadron_gN = dihadron;
        dihadron_gN.Boost(gNBoostNeg);
        pt_gN = Ptfunc(dihadron_gN);
        pt_gN_plus = Ptfunc(lv_p1_gN);
        pt_gN_minus = Ptfunc(lv_p2_gN);
        
        //Need target in gN
        target_gN = init_target;
        target_gN.Boost(gNBoostNeg);
        
        //Need partonic in gN
        ki_gN = ki;
        ki_gN.Boost(gNBoostNeg);
        
        kf_gN = kf;
        kf_gN.Boost(gNBoostNeg);
        
        //Feynman x
        xFpiplus = xFfunc(lv_p1_gN,lv_q_gN,W);
        xFpiminus = xFfunc(lv_p2_gN,lv_q_gN,W);
        
        //nu and W
        nu = nufunc(electron_beam_energy,electron.E);
        W = Wfunc(Q2,protonMass,nu);
        

        // Breit Frame Kinematics for delta k
        Breit = q;
        Breit_target.SetPxPyPzE(0,0,0,2 * x *protonMass); // E^2 = M^2 + P^2 --> P = 0 so E = M = 2 * x * protonmass
        Breit += Breit_target;
        BreitBoost = Breit.BoostVector();
        BreitBoost = -1 * BreitBoost;
            
        //Setting up delta k variables
        kfBreit = kf;
        kfBreit.Boost(BreitBoost);
        kfBreitTran = PtVectfunc(kfBreit); //breit frame boostkfbT in delta k calculation - needs to be a transverse light cone vector of form (V_x, V_y)
        
        q_Breit = q;
        q_Breit.Boost(BreitBoost);
//         Printf("q_Breit: "); q_Breit.Print(); cout << "\n";
        proton_Breit = proton.lv;
        proton_Breit.Boost(BreitBoost);
//         Printf("proton_Breit: ");
//         proton_Breit.Print(); cout << "\n";
        
        
        dihadronBreit = dihadron;
        dihadronBreit.Boost(BreitBoost);
//         Printf("DihadronBreit: ");
//         dihadronBreit.Print(); cout << "\n";
        dihadronBreitTran = PtVectfunc(dihadronBreit); //PBbT in qT part of delta k calculation
        
        plusBreit = piplus.lv;
        plusBreit.Boost(BreitBoost);
        plusBreitTran = PtVectfunc(plusBreit);
        minusBreit = piminus.lv;
        minusBreit.Boost(BreitBoost);
        minusBreitTran = PtVectfunc(minusBreit);
        
        PFFrame = q + init_target;
        PFBoost = PFFrame.BoostVector();
        PFBoost = -1 * PFBoost;
        qPF = q;
        qPF.Boost(PFBoost);
        qPFVect = qPF.Vect();
        qPFVectUnit = qPFVect.Unit();
        PFAngle = qPFVectUnit.Angle(zAxis);
        PFAxis = qPFVectUnit.Cross(zAxis);
        //To rotate -> vector.Rotate(PFAngle,PFAxis);
        
        //Hadron frame kinematics for q_T
        //Note: hadron frame here refers to frame where the target proton and outgoing hadron are back to back
        //and the target proton has the same four momentum as it has in the breit frame
        Hadron_frame = dihadron;
        Hadron_frame += Breit_target;
        HadronBoost = Hadron_frame.BoostVector();
        HadronBoost = -1 * HadronBoost;
        q_hadron = q;
        q_hadron.Boost(HadronBoost);
        q_T_hadron = PtVectfunc(q_hadron);
        
        q_T_lab = PtVectfunc(q);
        
        qTQ_lab = Ptfunc(q_T_lab) / sqrt(Q2);
        qTQ_hadron = Ptfunc(q_T_hadron) / sqrt(Q2);
        
        //
        //Photon Frame
        //
        
        //Dihadron
        dihadronPF = dihadron;
        dihadronPF.Boost(PFBoost);
        dihadronPF.Rotate(PFAngle,PFAxis);
        dihadronPFMinus = LightConeMinus(dihadronPF);
        
        plusPF = piplus.lv;
        plusPF.Boost(PFBoost);
        plusPF.Rotate(PFAngle,PFAxis);
        plusPFMinus = LightConeMinus(plusPF);
        minusPF = piminus.lv;
        minusPF.Boost(PFBoost);
        minusPF.Rotate(PFAngle,PFAxis);
        minusPFMinus = LightConeMinus(minusPF);
        //Virtual Photon
        qPF.Rotate(PFAngle,PFAxis);
        qPFMinus = LightConeMinus(qPF);
        //z_N and q_T
        z_N = dihadronPFMinus / qPFMinus;
        z_Nplus = plusPFMinus / qPFMinus;
        z_Nminus = minusPFMinus / qPFMinus;
        q_T = -1 * dihadronBreitTran / z_N;
        q_Tplus = -1 * plusBreitTran / z_Nplus;
        q_Tminus = -1 * minusBreitTran / z_Nminus;
        
        q_T_frac = dihadronBreitTran / z_h;
        qTQfrac = Ptfunc(q_T_frac) / sqrt(Q2);
        

        //q_T / Q for plotting
        qTQ = Ptfunc(q_T) / sqrt(Q2);
        q_TdivQplus = Ptfunc(q_Tplus) / sqrt(Q2);
        q_TdivQminus = Ptfunc(q_Tminus) / sqrt(Q2);
        

        //ki, k, and delta k
        deltak = kfBreitTran - (-1 * z_N * q_T); 
        
        k = kf - q;
        k_gN = k;
        k_gN.Boost(gNBoostNeg);
//         These ratios are calculated in lab frame
//        R0 = R0func(ki, kf, deltak, Q2);
//        R1 = R1func(dihadron, ki, kf);
//        R2 = R2func(k, Q2);
        
        //Ratios in gN frame
        R0 = R0func(ki_gN, kf_gN, deltak, Q2);
        kix = ki_gN.Px();
        kiy = ki_gN.Py();
        kiz = ki_gN.Pz();
        kiE = ki_gN.E();
        
        kfx = kf_gN.Px();
        kfy = kf_gN.Py();
        kfz = kf_gN.Pz();
        kfE = kf_gN.E();
        
        kTx = deltak_gN.Px();
        kTy = deltak_gN.Py();
        double ki2 = abs(ki_gN * ki_gN);
        double kf2 = abs(kf_gN * kf_gN);
        double deltak2 = abs(deltak * deltak);
        if(deltak2 > ki2 && deltak2 > kf2) {
            R0check = 0;//DeltaK is biggest
        }
        else if(ki2 > kf2) {
            R0check = 1;//ki is biggest
        }
        else {
            R0check = 2;//kf is biggest
        }
        
        R1 = R1func(dihadron_gN, ki_gN, kf_gN);
        ki_Breit = ki;
        ki_Breit.Boost(BreitBoost);
        kf_Breit = kf;
        kf_Breit.Boost(BreitBoost);
        k_Breit = k;
        k_Breit.Boost(BreitBoost);
        
        R2 = R2func(k_gN, Q2);
        xF = xFpiplus + xFpiminus;
        

        //Missing mass
        if(Mx <= 1.5) {
            continue;
        }
        
        //Feynman x
        if(xFpiplus <= 0 || xFpiminus <= 0) {
            continue;
        }
        
        //Vertex Position
        if(abs(electron.vz - piplus.vz) >= 20) {
            continue;
        }
        if(abs(electron.vz - piminus.vz) >= 20) {
            continue;
        }
        if(electron.vz <= -8 || electron.vz >= 3) {
            continue;
        }
        
        //Channel Selection
        
        //Virtual photon mass / momentum transfer
        if(Q2 <= 1 || Q2 >= 100) {
            continue;
        }
        //Hadronic system mass
        if(W <= 2 || W >= 100) {
            continue;
        }
        //Energy fraction
        if(y <= 0 || y >= 0.8) {
            continue;
        }
        if(piplus.P <= 1.25 || piminus.P <= 1.25) {
            continue;
        }

        tree_count += 1;
        tree_MC->Fill();
//         t_plus->Fill();
//         t_minus->Fill();
        //Need: x, z, Q2, pT, R0, R1, R2
        //zbins:
	
        for(int i = 0; i < zbins.size(); i++) {
            if(z_h <= zbins[i]) {
                zbinv[i].zFillVectors(x, Q2, pt_gN, R0, R1, R2);
                break;
            }
        }
        //Mh bins
        for(int i = 0; i < Mhbins.size(); i++) {
            if(Mdihadron <= Mhbins[i]) {
                Mhbinv[i].mhFillVectors(x, z_h, Q2, pt_gN, R0, R1, R2);
                break;
            }
        }
        //Q2 bins
        for(int i = 0; i < Q2bins.size(); i++) {
            if(Q2 <= Q2bins[i]) {
                Q2binv[i].Q2FillVectors(x, z_h, pt_gN, R0, R1, R2);
                break;
            }
        }
        //qTQ bins
        for(int i = 0; i < qTQbins.size(); i++) {
            if(qTQ_hadron <= qTQbins[i]) {
                qTQbinv[i].qTQFillVectors(x, z_h, Q2, pt_gN, R0, R1, R2);
                break;
            }
        }
        //x bins
        for(int i = 0; i < xbins.size(); i++) {
            if(x <= xbins[i]) {
                xbinv[i].xFillVectors(z_h, Q2, pt_gN, R0, R1, R2);
                break;
            }
        }
        //print out tree count every 100 to give update to user
        if(tree_count % 100 == 0) {
	//            cout << "Tree_count: " << tree_count << '\n';
        }
	
    }
    cout << "\033[0m" << "\033[49m";
    cout << "Final tree_count: " << tree_count << '\n';
    
    //Making new Affinity trees
    TTree *t_z_h = new TTree("tree_z_h_bins","Tree with mean values binned by z_h affinity calculations");
    TTree *t_x = new TTree("tree_x_bins","Tree with mean values binned by x affinity calculations");
    TTree *t_Mh = new TTree("tree_Mh_bins","Tree with mean values binned by Mh affinity calculations");
    TTree *t_Q2 = new TTree("tree_Q2_bins","Tree with mean values binned by Q2 affinity calculations");
    TTree *t_qTQ = new TTree("tree_qTQ_bins","Tree with mean values binned by Q2 affinity calculations");
    
    
    string infoString;
    Double_t z_h_t;
    Double_t x_t;
    Double_t Q2_t;
    Double_t pT_t;
    Double_t R0_t;
    Double_t R1_t;
    Double_t R2_t;
    
    t_z_h->Branch("Name",&infoString);
    t_z_h->Branch("x", &x_t);
    t_z_h->Branch("Q2", &Q2_t);
    t_z_h->Branch("pT", &pT_t);
    t_z_h->Branch("R0", &R0_t);
    t_z_h->Branch("R1", &R1_t);
    t_z_h->Branch("R2", &R2_t);
    
    t_x->Branch("Name",&infoString);
    t_x->Branch("z_h", &z_h_t);
    t_x->Branch("Q2", &Q2_t);
    t_x->Branch("pT", &pT_t);
    t_x->Branch("R0", &R0_t);
    t_x->Branch("R1", &R1_t);
    t_x->Branch("R2", &R2_t);
    
    t_Mh->Branch("Name",&infoString);
    t_Mh->Branch("x", &x_t);
    t_Mh->Branch("z_h", &z_h_t);
    t_Mh->Branch("Q2", &Q2_t);
    t_Mh->Branch("pT", &pT_t);
    t_Mh->Branch("R0", &R0_t);
    t_Mh->Branch("R1", &R1_t);
    t_Mh->Branch("R2", &R2_t);
    
    t_Q2->Branch("Name",&infoString);
    t_Q2->Branch("x", &x_t);
    t_Q2->Branch("z_h", &z_h_t);
    t_Q2->Branch("Q2", &Q2_t);
    t_Q2->Branch("pT", &pT_t);
    t_Q2->Branch("R0", &R0_t);
    t_Q2->Branch("R1", &R1_t);
    t_Q2->Branch("R2", &R2_t);
    
    t_qTQ->Branch("Name",&infoString);
    t_qTQ->Branch("x", &x_t);
    t_qTQ->Branch("z_h", &z_h_t);
    t_qTQ->Branch("Q2", &Q2_t);
    t_qTQ->Branch("pT", &pT_t);
    t_qTQ->Branch("R0", &R0_t);
    t_qTQ->Branch("R1", &R1_t);
    t_qTQ->Branch("R2", &R2_t);
    
    //Calculating means
    //Setting zbin means
    for(int i = 0; i < vinfoString.size() - 1; i++) {
        zbinv[i].meanZ_h();
        infoString = vinfoString[i];
        x_t = zbinv[i].xmean;
        Q2_t = zbinv[i].Q2mean;
        pT_t = zbinv[i].pTmean;
        R0_t = zbinv[i].R0mean;
        R1_t = zbinv[i].R1mean;
        R2_t = zbinv[i].R2mean;
        t_z_h->Fill();
        }
    for(int i = 0; i < vinfoString.size() - 1; i++) {
        xbinv[i].meanx();
        infoString = vinfoString[i];
        z_h_t = xbinv[i].z_hmean;
        Q2_t = xbinv[i].Q2mean;
        pT_t = xbinv[i].pTmean;
        R0_t = xbinv[i].R0mean;
        R1_t = xbinv[i].R1mean;
        R2_t = xbinv[i].R2mean;
        t_x->Fill();
        }
    for(int i = 0; i < vinfoString.size() - 1; i++) {
        Mhbinv[i].meanmh();
        infoString = vinfoString[i];
        x_t = Mhbinv[i].xmean;
        z_h_t = Mhbinv[i].z_hmean;
        Q2_t = Mhbinv[i].Q2mean;
        pT_t = Mhbinv[i].pTmean;
        R0_t = Mhbinv[i].R0mean;
        R1_t = Mhbinv[i].R1mean;
        R2_t = Mhbinv[i].R2mean;
        t_Mh->Fill();
        }
    
    for(int i = 0; i < vinfoString.size(); i++) {
        Q2binv[i].meanQ2();
        infoString = vinfoString[i];
        x_t = Q2binv[i].xmean;
        z_h_t = Q2binv[i].z_hmean;
        Q2_t = Q2binv[i].Q2mean;
        pT_t = Q2binv[i].pTmean;
        R0_t = Q2binv[i].R0mean;
        R1_t = Q2binv[i].R1mean;
        R2_t = Q2binv[i].R2mean;
        t_Q2->Fill();
        }
    
    for(int i = 0; i < vinfoString.size() - 1; i++) {
        qTQbinv[i].meanqTQ();
        infoString = vinfoString[i];
        x_t = qTQbinv[i].xmean;
        z_h_t = qTQbinv[i].z_hmean;
        Q2_t = qTQbinv[i].Q2mean;
        pT_t = qTQbinv[i].pTmean;
        R0_t = qTQbinv[i].R0mean;
        R1_t = qTQbinv[i].R1mean;
        R2_t = qTQbinv[i].R2mean;
        t_qTQ->Fill();
        }
    
    f->Write();
    delete f;
    
    return 0;
}
