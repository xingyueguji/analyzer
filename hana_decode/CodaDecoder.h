#ifndef Podd_CodaDecoder_h_
#define Podd_CodaDecoder_h_

/////////////////////////////////////////////////////////////////////
//
//   CodaDecoder
//
//           Object Oriented version of decoder
//           Sept, 2014    R. Michaels
//
/////////////////////////////////////////////////////////////////////

#include "THaEvData.h"
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace Decoder {

class CodaDecoder : public THaEvData {
public:
  CodaDecoder();
  virtual ~CodaDecoder();

  virtual Int_t  Init();

  virtual Int_t  LoadEvent(const UInt_t* evbuffer);

  virtual UInt_t GetPrescaleFactor( UInt_t trigger ) const;
  virtual void   SetRunTime( ULong64_t tloc );
  virtual Int_t  SetDataVersion( Int_t version ) { return SetCodaVersion(version); }
          Int_t  SetCodaVersion( Int_t version );

  virtual Bool_t DataCached() { return fMultiBlockMode && !fBlockIsDone; }
  virtual Int_t  LoadFromMultiBlock();
          Bool_t IsMultiBlockMode() const { return fMultiBlockMode; };
          Bool_t BlockIsDone() const { return fBlockIsDone; };

  virtual Int_t  FillBankData( UInt_t* rdat, UInt_t roc, Int_t bank,
                               UInt_t offset = 0, UInt_t num = 1 ) const;

  UInt_t         GetTSEvtype() const { return tsEvType; }

  enum { MAX_PSFACT = 12 };

  // CODA file format exception, thrown by LoadEvent/LoadFromMultiBlock
  class coda_format_error : public std::runtime_error {
  public:
    explicit coda_format_error( const std::string& what_arg )
      : std::runtime_error(what_arg) {}
    explicit coda_format_error( const char* what_arg )
      : std::runtime_error(what_arg) {}
  };

protected:
  virtual Int_t  LoadIfFlagData(const UInt_t* evbuffer);

  Int_t FindRocs(const UInt_t *evbuffer);  // CODA2 version
  Int_t FindRocsCoda3(const UInt_t *evbuffer); // CODA3 version
  Int_t roc_decode( UInt_t roc, const UInt_t* evbuffer, UInt_t ipt, UInt_t istop );
  Int_t bank_decode( UInt_t roc, const UInt_t* evbuffer, UInt_t ipt, UInt_t istop );
  Int_t physics_decode( const UInt_t* evbuffer );

  void CompareRocs();
  void ChkFbSlot( UInt_t roc, const UInt_t* evbuffer, UInt_t ipt, UInt_t istop );
  void ChkFbSlots();

  virtual Int_t  init_slotdata();
  virtual Int_t  interpretCoda3( const UInt_t* buffer );
  virtual Int_t  trigBankDecode( const UInt_t* evbuffer );
  Int_t daqConfigDecode( const UInt_t* evbuf );   //FIXME: BCI: make virtual
  Int_t prescale_decode_coda2( const UInt_t* evbuffer );
  Int_t prescale_decode_coda3( const UInt_t* evbuffer );
  void  dump( const UInt_t* evbuffer ) const;
  void  debug_print( const UInt_t* evbuffer ) const;

  // Data
  UInt_t nroc;
  std::vector<UInt_t> irn;
  std::vector<bool>   fbfound;
  std::vector<UInt_t> psfact;
  Bool_t buffmode,synchmiss,synchextra,fdfirst;
  Int_t  chkfbstat;

  class BankDat_t {            // Coordinates of bank data in raw event
  public:
    BankDat_t() : key(kMaxUInt), pos(0), len(0) {}
    BankDat_t( UInt_t key, UInt_t pos, UInt_t len ) : key(key), pos(pos), len(len) {}
    bool operator==( const BankDat_t& rhs ) const { return key == rhs.key; }
    bool operator==( UInt_t key_to_find )   const { return key == key_to_find; }
    bool operator< ( const BankDat_t& rhs ) const { return key <  rhs.key; }
    UInt_t key;   // bank number + (roc << 16)
    UInt_t pos;   // position in evbuffer[]
    UInt_t len;   // length of data including pos, so pos+len-1 = last word of data
  };
  std::vector<BankDat_t> bankdat;

  // CODA3 stuff
  UInt_t evcnt_coda3;
  Bool_t fMultiBlockMode, fBlockIsDone;
  UInt_t tsEvType, bank_tag, block_size;

  class TBOBJ {
  public:
     TBOBJ() : blksize(0), tag(0), nrocs(0), len(0), tsrocLen(0), evtNum(0),
               runInfo(0), start(nullptr), evTS(nullptr), evType(nullptr),
               TSROC(nullptr) {}
     void     Clear() { memset(this, 0, sizeof(*this)); }
     uint32_t Fill( const uint32_t* evbuffer, uint32_t blkSize, uint32_t tsroc );
     bool     withTimeStamp()   const { return (tag & 1) != 0; }
     bool     withRunInfo()     const { return (tag & 2) != 0; }
     bool     withTriggerBits() const { return (tsrocLen > 2);}

     uint32_t blksize;          /* total number of triggers in the Bank */
     uint16_t tag;              /* Trigger Bank Tag ID = 0xff2x */
     uint16_t nrocs;            /* Number of ROC Banks in the Event Block (val = 1-256) */
     uint32_t len;              /* Total Length of the Trigger Bank - including Bank header */
     uint32_t tsrocLen;         /* Number of words in TSROC array */
     uint64_t evtNum;           /* Starting Event # of the Block */
     uint64_t runInfo;          /* Run Info Data (optional) */
     const uint32_t *start;     /* Pointer to start of the Trigger Bank */
     const uint64_t *evTS;      /* Pointer to the array of Time Stamps (optional) */
     const uint16_t *evType;    /* Pointer to the array of Event Types */
     const uint32_t *TSROC;     /* Pointer to Trigger Supervisor ROC segment data */
   };

   TBOBJ tbank;

   ClassDef(CodaDecoder,0) // Decoder for CODA event buffer
};

}

#endif
