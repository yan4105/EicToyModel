//
// AYK (ayk@bnl.gov), 2014/08/06
//
//  GEM geometry description file;
//

#include <iostream>
using namespace std;

#include <TGeoTube.h>
#include <TGeoVolume.h>
#include <TGeoMatrix.h>
#include <TGeoTrd1.h>
#include <TGeoArb8.h>
#include <TGeoPara.h>

#include <EtmOrphans.h>
#include <GemGeoParData.h>

// ---------------------------------------------------------------------------------------

GemModule::GemModule( void ):
  mActiveWindowBottomWidth    (   30.0 * etm::mm),
  mActiveWindowTopWidth       (  430.0 * etm::mm),
  mActiveWindowHeight         (  700.0 * etm::mm),

  // Frame parameters accoring to Kondo's sbsCrossSection.pdf file),
  mFrameThickness             (   18.0 * etm::mm),
  mFrameBottomEdgeWidth       (   30.0 * etm::mm),
  mFrameTopEdgeWidth          (   30.0 * etm::mm),
  mFrameSideEdgeWidth         (    8.0 * etm::mm),
  
  // FIXME: put aluminum layer later as well),
  mEntranceWindowMaterial     ( "GemKapton"),
  mEntranceWindowThickness    (   50.0 * etm::um),
  
  // Use evaristo.pdf p.10 for the foil parameters:
  //  - drift foil    : 50um kapton + 3um copper)
  //  - GEM foil      : 30um kapton + 3um copper (80% area fraction))
  //  - readout foils : 30um kapton + 3um copper total)
  mDriftFoilKaptonThickness   (   50.0 * etm::um),
  mDriftFoilCopperThickness   (    3.0 * etm::um),
  mGemFoilAreaFraction        (   0.80),
  mGemFoilKaptonThickness     (   30.0 * etm::um),
  mGemFoilCopperThickness     (    3.0 * etm::um),
  
  mReadoutG10Thickness        (    0.0 * etm::mm),
  mReadoutKaptonThickness     (   30.0 * etm::um), 
  mReadoutCopperThickness     (    3.0 * etm::um),
  
// 3mm thick Nomex honeycomb for SBS GEMs),
  mReadoutSupportMaterial     ( "GemNomex"),
  mReadoutSupportThickness    (    3.0 * etm::mm),
  
// FIXME: check on that!),
  mGasMixture                 ( "arco27030"),
  
  mEntranceRegionLength       (    3.0 * etm::mm),
  mDriftRegionLength          (    3.0 * etm::mm),
// Assume triple GEM layout),
  mFirstTransferRegionLength  (    2.0 * etm::mm),
  mSecondTransferRegionLength (    2.0 * etm::mm),
  mInductionRegionLength      (    2.0 * etm::mm)
{
} // GemModule::GemModule()
  
// ---------------------------------------------------------------------------------------

int GemGeoParData::ConstructGeometry(bool root, bool gdml, bool check)
{
  const char *detName = mDetName->Name().Data();

  // Loop through all wheels (or perhaps single modules) independently;
  for(unsigned wl=0; wl<mWheels.size(); wl++) {
    GemWheel *wheel   = mWheels[wl];
    GemModule *module = wheel->mModule;

    // Assume top side is wider;
    double sideSlope = atan((module->mActiveWindowTopWidth - module->mActiveWindowBottomWidth)/
			    (2*module->mActiveWindowHeight));

    double moduleContainerHeight;
    TGeoVolume *vwcontainer, *vmcontainer;

    // Figure out parameters of the wheel container (air) volume first; 
    {
      double thickness = (wheel->mModuleNum == 1 ? 1 : 2)*module->mFrameThickness + 
	mMountingRingBeamLineThickness;
    
      double minRadius = wheel->mRadius - module->mActiveWindowHeight/2 - 
	module->mFrameBottomEdgeWidth;
      // Can perfectly be for a configuration with a single module FLYSUB "wheel";
      if (minRadius < 0.0) minRadius = 0.0;

      // Assume frame side width is given in a section parallel to the base; 
      double xx = module->mActiveWindowTopWidth/2 + module->mFrameTopEdgeWidth*tan(sideSlope) + 
	module->mFrameSideEdgeWidth;
      double yy = wheel->mRadius + module->mActiveWindowHeight/2 + module->mFrameTopEdgeWidth;
      double maxRadius = sqrt(xx*xx + yy*yy);

      char wheelContainerVolumeName[128];
      snprintf(wheelContainerVolumeName, 128-1, "%sWheelContainerVolume%02d", detName, wl);
      
      TGeoTube *wcontainer = new TGeoTube(wheelContainerVolumeName,
					  minRadius,
					  maxRadius,
					  thickness/2);
      vwcontainer = new TGeoVolume(wheelContainerVolumeName, wcontainer, GetMedium(_AIR_));
		  
      GetTopVolume()->AddNode(vwcontainer, 0, wheel->mTransformation);
    }

    // Module container (air) volume; here can indeed use TRD1 volume;
    {
      char moduleContainerVolumeName[128];
      snprintf(moduleContainerVolumeName, 128-1, "%sModuleContainerVolume%02d", detName, wl);

      moduleContainerHeight = module->mFrameTopEdgeWidth + module->mFrameBottomEdgeWidth +
	module->mActiveWindowHeight;

      TGeoTrd1 *mcontainer = new TGeoTrd1(moduleContainerVolumeName,
					  module->mActiveWindowBottomWidth/2 + module->mFrameSideEdgeWidth - 
					  module->mFrameBottomEdgeWidth*tan(sideSlope),
					  module->mActiveWindowTopWidth/2 + module->mFrameSideEdgeWidth + 
					  module->mFrameTopEdgeWidth*tan(sideSlope),
					  module->mFrameThickness/2,
					  moduleContainerHeight/2);
      vmcontainer = new TGeoVolume(moduleContainerVolumeName, mcontainer, GetMedium(_AIR_));

      // Place all the modules into the wheel container volume;
      for(unsigned md=0; md<wheel->mModuleNum; md++) {
	double effRadius = wheel->mRadius + (module->mFrameTopEdgeWidth - module->mFrameBottomEdgeWidth)/2;

	TGeoRotation *rw = new TGeoRotation();
	double degAngle = md*360.0/wheel->mModuleNum;
	double radAngle = degAngle*TMath::Pi()/180.0;
	rw->SetAngles(90.0, 0.0 - degAngle, 180.0,  0.0, 90.0, 90.0 - degAngle);
	
	double xOffset = effRadius*sin(radAngle);
	double yOffset = effRadius*cos(radAngle);
	double zOffset = wheel->mModuleNum == 1 ? 0.0 : 
	  (md%2 ? -1.0 : 1.0)*(module->mFrameThickness + mMountingRingBeamLineThickness)/2;
	
	vwcontainer->AddNode(vmcontainer, md, new TGeoCombiTrans(xOffset, yOffset, zOffset, rw));
      } //for md
    }

    // And now put the other stuff piece by piece; unfortunately have to cook frame 
    // out of 4 pieces rather than a single TRD1, since otherwise drawing will 
    // become a nightmare (ROOT seems to be not able to draw inner walls at the
    // border of volume and its subvolume in a reasonable way);
    {
      //
      // A trapezoid shape; indeed could use TRD1 inner volumes and rotate them
      // accordingly; the trouble is that then I'd have to screw up local
      // coordinate system of the sensitive volume (so Z will be pointing in 
      // radial direction rather than along the beam line; consider to use
      // TGeoArb8 shape for this reason; CHECK: is there a performance penalty?;
      //

      char bottomFrameEdgeName[128], topFrameEdgeName[128], sideFrameEdgeName[128];

      // Want them to start with the same name pattern;
      snprintf(bottomFrameEdgeName, 128-1, "%sFrameEdgeBottom%02d", detName, wl);
      snprintf(topFrameEdgeName,    128-1, "%sFrameEdgeTop%02d",    detName, wl);
      snprintf(sideFrameEdgeName,   128-1, "%sFrameEdgeSide%02d",   detName, wl);
      
      // Bottom edge; here I can indeed use TRD1 volume;
      {
	TGeoTrd1 *bottom = new TGeoTrd1(bottomFrameEdgeName,
					module->mActiveWindowBottomWidth/2 - 
					module->mFrameBottomEdgeWidth*tan(sideSlope),
					module->mActiveWindowBottomWidth/2,
					module->mFrameThickness/2,
					module->mFrameBottomEdgeWidth/2);
	TGeoVolume *vbottom = new TGeoVolume(bottomFrameEdgeName, bottom, GetMedium(mG10Material));
	
	double zOffset = -(moduleContainerHeight - module->mFrameBottomEdgeWidth)/2;
	vmcontainer->AddNode(vbottom, 0, new TGeoCombiTrans(0.0, 0.0, zOffset, 0));
      }
      // Top edge; the same;
      {
	TGeoTrd1 *top = new TGeoTrd1(topFrameEdgeName,
				     module->mActiveWindowTopWidth/2,
				     module->mActiveWindowTopWidth/2 +
				     module->mFrameTopEdgeWidth*tan(sideSlope),
				     module->mFrameThickness/2,
				     module->mFrameTopEdgeWidth/2);
	TGeoVolume *vtop = new TGeoVolume(topFrameEdgeName, top, GetMedium(mG10Material));
	
	double zOffset = (moduleContainerHeight - module->mFrameTopEdgeWidth)/2;
	vmcontainer->AddNode(vtop, 0, new TGeoCombiTrans(0.0, 0.0, zOffset, 0));
      }

      // A pair of side edges; TGeoPara will do;
      TGeoPara *side = new TGeoPara(sideFrameEdgeName,
				    module->mFrameSideEdgeWidth/2,
				    module->mFrameThickness/2,
				    moduleContainerHeight/2,
				    0.0, sideSlope*180/TMath::Pi(), 0.0);
      TGeoVolume *vside = new TGeoVolume(sideFrameEdgeName, side, GetMedium(mG10Material));
      for(unsigned lr=0; lr<2; lr++) {
	double xOffset = (lr ? -1.0 : 1.)*
	  (module->mActiveWindowBottomWidth/2 - module->mFrameBottomEdgeWidth*tan(sideSlope) + 
	   module->mFrameSideEdgeWidth/2 + (moduleContainerHeight/2)*tan(sideSlope));

	TGeoRotation *rw = 0;
	if (lr) {
	  rw = new TGeoRotation();
	  rw->RotateZ(180.0);
	} //if

	vmcontainer->AddNode(vside, lr, new TGeoCombiTrans(xOffset, 0.0, 0.0, rw));
      } //for lr

      // Eventually populate single layers, one by one;
      {
	double yOffset = -module->mFrameThickness/2;// + 0.100;

	//
	// XY-projection shape does not change; it's only zOffset, thickness and material;
	// 
	// Proceed in direction opposite to the incident particles; whatever
	// is left in thickness will remain air in front of the entrance foil;
	//
	// This readout support stuff is optionally present;
	if (module->mReadoutSupportThickness)
	  PlaceMaterialLayer(detName, "ReadoutSupport", wl, vmcontainer, 
			     module->mReadoutSupportMaterial.Data(), 
			     module->mReadoutSupportThickness, &yOffset); 
	if (module->mReadoutG10Thickness)
	  PlaceMaterialLayer(detName, "ReadoutG10", wl, vmcontainer, 
			     mG10Material.Data(), 
			     module->mReadoutG10Thickness, &yOffset); 

	// Readout foil is always there;
	PlaceMaterialLayer(detName, "ReadoutKapton", wl, vmcontainer, 
			   mKaptonMaterial.Data(),
			   module->mReadoutKaptonThickness, &yOffset); 

	// Copper layers are extremely thin -> put one effective layer;
	{
	  double thickness = module->mReadoutCopperThickness +  
	    // 3x kapton layer, double-sided metallization;
	    3*2*module->mGemFoilAreaFraction*module->mGemFoilCopperThickness +
	    module->mDriftFoilCopperThickness;

	  PlaceMaterialLayer(detName, "EffectiveCopper", wl, vmcontainer, 
			     _COPPER_, thickness, &yOffset); 
	}

	// Induction region;
	{
	  PlaceMaterialLayer(detName, "InductionRegionGas", wl, vmcontainer, 
			     module->mGasMixture.Data(), 
			     module->mInductionRegionLength, &yOffset);

	  PlaceMaterialLayer(detName, "InductionRegionFoil", wl, vmcontainer, 
			     mKaptonMaterial.Data(), 
			     module->mGemFoilAreaFraction*module->mGemFoilKaptonThickness, 
			     &yOffset); 
	}

	// 2-d transfer region;
	{
	  PlaceMaterialLayer(detName, "SecondTransferRegionGas", wl, vmcontainer, 
			     module->mGasMixture.Data(), 
			     module->mSecondTransferRegionLength, &yOffset);

	  PlaceMaterialLayer(detName, "SecondTransferRegionFoil", wl, vmcontainer, 
			     mKaptonMaterial.Data(), 
			     module->mGemFoilAreaFraction*module->mGemFoilKaptonThickness, 
			     &yOffset); 
	}

	// 1-st transfer region;
	{
	  PlaceMaterialLayer(detName, "FirstTransferRegionGas", wl, vmcontainer, 
			     module->mGasMixture.Data(), 
			     module->mFirstTransferRegionLength, &yOffset);

	  PlaceMaterialLayer(detName, "FirstTransferRegionFoil", wl, vmcontainer, 
			     mKaptonMaterial.Data(), 
			     module->mGemFoilAreaFraction*module->mGemFoilKaptonThickness, 
			     &yOffset); 
	}

	// drift region;
	{
	  // NB: this is the sensitive volume!;
	  PlaceMaterialLayer(detName, "DriftRegionGas", wl, vmcontainer, 
			     module->mGasMixture.Data(), 
			     module->mDriftRegionLength, &yOffset);

	  PlaceMaterialLayer(detName, "DriftRegionFoil", wl, vmcontainer, 
			     mKaptonMaterial.Data(), 
			     module->mDriftFoilKaptonThickness, &yOffset); 
	}

	// entrance region;
	{
	  PlaceMaterialLayer(detName, "EntranceRegionGas", wl, vmcontainer, 
			     module->mGasMixture.Data(), 
			     module->mEntranceRegionLength, &yOffset);

	  PlaceMaterialLayer(detName, "EntranceWindow", wl, vmcontainer, 
			     module->mEntranceWindowMaterial.Data(), 
			     module->mEntranceWindowThickness, &yOffset); 
	}
      }
    } //if

    {
      AddLogicalVolumeGroup(wheel->mModuleNum);
      // Yes, carelessly create one map per layer;
      EicGeoMap *fgmap = CreateNewMap();

      // FIXME: do it better later;
      char volumeName[128];
      snprintf(volumeName, 128-1, "%s%s%02d", detName, "DriftRegionGas", wl);
      fgmap->AddGeantVolumeLevel(volumeName,   0);

      fgmap->SetSingleSensorContainerVolume(volumeName);

      snprintf(volumeName, 128-1, "%sModuleContainerVolume%02d", detName, wl);
      fgmap->AddGeantVolumeLevel(volumeName,   wheel->mModuleNum);

      for(unsigned md=0; md<wheel->mModuleNum; md++) {
	  UInt_t geant[2] = {0, md}, logical[1] = {md};

	  if (SetMappingTableEntry(fgmap, geant, wl, logical)) {
	    cout << "Failed to set mapping table entry!" << endl;
	    exit(0);
	  } //if
	} //for md
    }
  } //for wl

  GetColorTable()         ->AddPatternMatch("FrameEdge",      kGray);
  GetColorTable()         ->AddPatternMatch("EntranceWindow", kOrange);
  if (mTransparency)
    GetTransparencyTable()->AddPatternMatch("EntranceWindow", mTransparency);
  GetColorTable()         ->AddPatternMatch("ReadoutSupport", kOrange);
  if (mTransparency)
    GetTransparencyTable()->AddPatternMatch("ReadoutSupport", mTransparency);

  // Place this stuff as a whole into the top volume and write out;
  FinalizeOutput(root, gdml, check);

  return 0;
} // GemGeoParData::ConstructGeometry()

// ---------------------------------------------------------------------------------------

void GemGeoParData::PlaceMaterialLayer(const char *detName, const char *volumeNamePrefix, 
				       unsigned wheelID, 
				       TGeoVolume *moduleContainer, const char *material, 
				       double thickness, double *yOffset)
{
  char volumeName[128];
  GemWheel *wheel   = mWheels[wheelID];
  GemModule *module = wheel->mModule;

  snprintf(volumeName, 128-1, "%s%s%02d", detName, volumeNamePrefix, wheelID);
  
  // Well, be aware that with a switch from Arb8 (which is a problematic shape in 
  // many respects assuming a TGeo -> GDML ->TGeo conversion), this volume can not 
  // be easily used in the native EicRoot tracking since the coordinate system is
  // kind of wrong (Y & Z flipped);
  TGeoTrd1 *shape = new TGeoTrd1(volumeName,
				 module->mActiveWindowBottomWidth/2,
				 module->mActiveWindowTopWidth/2,
				 thickness/2,
				 module->mActiveWindowHeight/2);
  TGeoVolume *vshape = new TGeoVolume(volumeName, shape, GetMedium(material));

  double zOffset = -(module->mFrameTopEdgeWidth - module->mFrameBottomEdgeWidth)/2;
  moduleContainer->AddNode(vshape, 0, new TGeoCombiTrans(0.0, (*yOffset + thickness/2), 
							 zOffset, 0));

  *yOffset += thickness;
} // GemGeoParData::PlaceMaterialLayer()

// ---------------------------------------------------------------------------------------

ClassImp(GemModule)
ClassImp(GemWheel)
ClassImp(GemGeoParData)
