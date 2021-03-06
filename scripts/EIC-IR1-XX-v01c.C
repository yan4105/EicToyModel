
{
  auto eic = new EicToyModel();

  // Shift IP if needed; define canvas width; request eta=0 line in the drawing; set name;
  eic->ip(0.0 * etm::cm)->width(1200)->AddEtaLine(0.0)->SetName("EIC-IR1-XX-v01c");
  eic->ir(1020.0 * etm::cm, 420.0*etm::cm);
  // Define acceptance ranges and the vacuum chamber design;
  eic->acceptance(-4.2, -1.0, 1.2, 4.2);
  eic->SetAzimuthalSegmentation(12);
  eic->DefineVacuumChamber(new vc2020_03_20());
  //eic->DrawIP6boundaries();
  eic->UseDetectorHighlighting();

  // Vertex tracker;
  {
    auto vtx = eic->vtx(); vtx->offset(0.0 * etm::cm);

    vtx->add("Si Tracker",20 * etm::cm);//->highlight();
  }

  // Barrel;
  {
    auto mid = eic->mid(); mid->offset( 20 * etm::cm);
      
    //mid->add("TRACKER",   75 * etm::cm);//->highlight();
    //mid->add("Si Tracker",75 * etm::cm);//->highlight(0.4);
    mid->add("MPGD",   75 * etm::cm);//->highlight(0.4);
    //mid->add("TPC",       75 * etm::cm);//->highlight();
    mid->add("Cherenkov", 25 * etm::cm);//->highlight();
    mid->add("EmCal",     30 * etm::cm);//->highlight(0.4);
    mid->add("Cryostat",  40 * etm::cm);
    mid->add("HCal",     120 * etm::cm)->brick();//->highlight();
  }

  // Hadron-going endcap;
  {
    auto fwd = eic->fwd(); fwd->offset(130 * etm::cm);

    //fwd->add("TRACKER",   15 * etm::cm)->brick();
    fwd->add("MPGD",      15 * etm::cm)->brick();//->highlight();
    fwd->marker();

    fwd->add("HM RICH",  150 * etm::cm)->trim(0.9, 1.0);//->highlight(0.4);
    //fwd->add("MPGD",      15 * etm::cm)->brick();//->highlight();//0.2);
    //fwd->add("sTGC",      15 * etm::cm)->brick();//->highlight(0.4);
    for(unsigned nn=0; nn<2; nn++)
      //for(unsigned nn=0; nn<1; nn++)
      fwd->add("TRD",     15 * etm::cm)->brick()->highlight();

    //fwd->add("Preshower", 10 * etm::cm);
    fwd->add("TOF",       10 * etm::cm);//->highlight();
    fwd->add("EmCal",     40 * etm::cm);//->highlight(0.4);
    fwd->add("HCal",     105 * etm::cm);//->highlight(0.4);
  } 

  // Electron-going endcap;
  {
    auto bck = eic->bck(); bck->offset(130 * etm::cm);

    //bck->add("TRACKER",   15 * etm::cm)->brick();
    bck->add("MPGD",      15 * etm::cm)->brick();//->highlight(0.4);
    for(unsigned nn=0; nn<3; nn++)
      bck->add("TRD",     15 * etm::cm)->brick()->highlight();
    bck->marker();

    bck->add("Cherenkov", 25 * etm::cm)->highlight(0.4);
    //bck->add("TOF",       10 * etm::cm)->brick()->highlight(0.5);
    bck->add("TOF",       10 * etm::cm)->brick();//->highlight();
    bck->add("Preshower",  5 * etm::cm)->brick();
    bck->add("EmCal",     50 * etm::cm);//->highlight();//0.2);
    bck->add("HCal",     105 * etm::cm);//->highlight();
  }

  // Declare eta boundary configuration;
  {
    eic->vtx()->get("Si Tracker")->stretch(eic->bck()->get("Cherenkov"));
    eic->vtx()->get("Si Tracker")->stretch(eic->fwd()->get("HM RICH"));

    //eic->mid()->get("TRACKER")   ->stretch(eic->bck()->get("TRACKER"));
    //eic->mid()->get("TRACKER")   ->stretch(eic->fwd()->get("TRACKER"));
    //eic->mid()->get("Si Tracker")   ->stretch(eic->bck()->get("MPGD"));
    //eic->mid()->get("Si Tracker")   ->stretch(eic->fwd()->get("MPGD"));
    //eic->mid()->get("TRACKER")   ->stretch(eic->bck()->get("MPGD"));
    //eic->mid()->get("TRACKER")   ->stretch(eic->fwd()->get("MPGD"));
    eic->mid()->get("MPGD")   ->stretch(eic->bck()->get("MPGD"));
    eic->mid()->get("MPGD")   ->stretch(eic->fwd()->get("MPGD"));
    //eic->mid()->get("TPC")   ->stretch(eic->bck()->get("MPGD"));
    //eic->mid()->get("TPC")   ->stretch(eic->fwd()->get("MPGD"));
    eic->mid()->get("HCal")      ->stretch(eic->bck()->get("HCal"));
    eic->mid()->get("HCal")      ->stretch(eic->fwd()->get("HCal"));
    eic->mid()->get("Cryostat")  ->stretch(eic->bck()->get("HCal"));
    eic->mid()->get("Cherenkov") ->stretch(eic->bck()->get("Cherenkov"));

    eic->fwd()->get("HCal")      ->stretch(eic->mid()->get("HCal"), 90 * etm::cm);

    eic->bck()->get("HCal")      ->stretch(eic->mid()->get("HCal"), 90 * etm::cm);
    eic->bck()->get("EmCal")     ->stretch(eic->mid()->get("EmCal"));
  }

  // Beautify picture a little bit;
  eic->ApplyStandardTrimming();

  // Draw horizontal cross cut view; write the .root file out;
  eic->hdraw();
  eic->write();
} 
