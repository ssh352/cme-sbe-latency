# CME Depth Book Construction

This application is designed to be run on a server with access to CME multicast
traffic.

## Build
```
docker build -t cme-sbe-latency .
```

## Usage

```
docker run cme-sbe-latency 0.0.0.0 <path_to_config.xml>
```

# Sample

```
docker run --net=host cme-sbe-latency  0.0.0.0 config.xml

 Security 23936, Recovering 0, LastSeqNum 2541935

BidV		Price		AskV
--------------------------------------
		258475		364
		258450		355
		258425		330
		258400		474
		258375		315
		258350		275
		258325		209
		258300		176
		258275		171
		258250		44
--------------------------------------
136		258225
186		258200
201		258175
186		258150
351		258125
256		258100
260		258075
292		258050
407		258025
428		258000


```
# Todo

Refactor items:
* add support for commit()
* manage subscriptions
* confirm this line : if(*message.securityGroup() != message.securityGroupNullValue()) { // need to confirm this
* not measuring message level seqnum gap , this means we could miss securitystatus, instrument definition if rptseq didnt gap too
* start adding catch2 tests today


* Items to pickup on:
    0. We are in a working state IF we join snapshot at a good time (seqnum must be low enough to capture our snapshot...)
    1. Recovery channels loop - so logic for sequence gap is not valid, might need to track primary lastseq and secondary lastseq to identify
        difference between 1,1,2,2,3,3,4,4,5,5,6,6,7,7,1,1,2,2... and 1,1,2,3,2,3 .... maybe accept seqnum of 1 as a reset? what about 1,2,1..
        certainly need to keep separate seqnum in this instance....
        
    2. Incremental feed - this is looking good
    3. Decoder needs to expose interface: 
        StartRecovery() // Important because decoder needs to know when to ignore 
                        // snapshot and when not to, including clearing book and setting rptseqnum
        StopRecovery() // maybe decoder sets this state and notifies

* Functionality
    1. Channel Reset
    2. Instrument Discovery
    3. Complete processing orders
    4. Support implied books
   
* Testing:
    0. Add Catch2 support
    1. Unit test:
        * Decoder
        * Subscriber 
    
* Performance:
    1. Valgrind
    2. Other???

* Record latency range for sample subscription to ES, compare to SBE adapter

* Recovery:

    1. Download all securities on startup from instrument replay feed
    2. Upon a specific subscription
    
        * download latest instrument contract information (market phase etc included in this)
        * download snapshot
        * download incremental
            * On Instrument contract download complete -> leave instrument channel()
            * On Incremental matching snapshot-> leave incremental channel()
           
    3. Remain on incremental feed
        * if seq gap identified -> join snapshot
                                -> join instrument
                                
            repeat until recovery()
                                
     