# related issues
someone else had the same discovery here:
      [x] https://qt-project.org/forums/viewthread/11763
      [x] http://www.qtcentre.org/threads/41649-QNetworkAccessManager-smart-handling-of-timeouts-network-errors
       ^
       \--- notified the thread owners of my minimal example 
   
# network connection

    the virtualbox based setup i used:

      inet 
        |
        |   eth0|---------|   virtualized    |---------|
        \ ------|  host   |------------------|  guest  |
                |---------| vboxnet0    eth1 |---------|
                          |
                        inside the guest
                          this interface is called eth1
                        
                        
## a similar setup without virtualization would be:
host pc ---- switch1 ---- switch2 ----- internet 

and then unplug switch2 so that networkmanager/ifplugd (or similar) on the host won't notice that the cable has been unplugged.

  
# working condition 
when the network connection is working properly i can see:
      guest % ./qt-QNetworkAccessManager-issues
      NetGet about to do lookupHost 
      lookupHost ID =  1 
      NetGet now invoking quit() at QCoreApplication::instance() 
      ------------------------------------------------------------------------------------------
        type the letter 'q' and hit RETURN to run quit(
      ------------------------------------------------------------------------------------------
      
      --------------------------------------------
      printResults (QHostAddress("87.238.53.172") )  
      -------------------------------------------- 
      canceling lookupHost ID =  1 
      ~NetGet 
      ./qt-QNetworkAccessManager-issues  0.00s user 0.01s system 6% cpu 0.177 total


# error condition
when i unplug the network cable (host::eth0), the virtualbox guest can't detect it. this behaviour is not specific
to virtualization and affects all similar setups. for example if you unplug the network cable between your router to the switch
you have your test machine connected to.

NOTE: beware of dhcp issues, check that your network adapter still has a valid lease or use
      statically configured addresses instead. 

in this example it took me about 70ms to press q + RETURN, see:

    guest % time ./qt-QNetworkAccessManager-issues
    NetGet about to do lookupHost 
    lookupHost ID =  1 
    NetGet now invoking quit() at QCoreApplication::instance() 
    ------------------------------------------------------------------------------------------
      type the letter 'q' and hit RETURN to run quit(
    ------------------------------------------------------------------------------------------
    
    q
    you typed:  "q" 
    input handling exited, program should quit in less than a second! 
    canceling lookupHost ID =  1 
    ~NetGet 
    ./qt-QNetworkAccessManager-issues  0.02s user 0.00s system 0% cpu 40.070 total

still i had to wait 40.070 seconds until the DNS hit a timeout!
             
## related tests

this means, if i ping 8.8.8.8 from the guest, it will send these packages but they will end in nirvana:

    guest % ping 8.8.8.8
    PING 8.8.8.8 (8.8.8.8) 56(84) bytes of data.
    From 192.168.56.101 icmp_seq=1 Destination Host Unreachable
    From 192.168.56.101 icmp_seq=2 Destination Host Unreachable
    From 192.168.56.101 icmp_seq=3 Destination Host Unreachable
    From 192.168.56.101 icmp_seq=4 Destination Host Unreachable
    From 192.168.56.101 icmp_seq=5 Destination Host Unreachable
    ...


    guest % ip a
    3: eth1: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc pfifo_fast state DOWN qlen 1000
        link/ether 08:00:27:09:8c:39 brd ff:ff:ff:ff:ff:ff
        inet 192.168.56.101/24 brd 192.168.56.255 scope global eth1
        inet6 fe80::a00:27ff:fe09:8c39/64 scope link 
          valid_lft forever preferred_lft forever

    guest % ip r 
    default via 192.168.56.1 dev eth1 
    192.168.56.0/24 dev eth1  proto kernel  scope link  src 192.168.56.101 

  
# what causes the stall?
this is a DNS timeout of 40 seconds and i can reproduce this by modifying /etc/resolv.conf.

say you put:
  'nameserver 127.0.0.1' 
and there is no resolver running on localhost i get an immediate timeout instead of waiting forever. 

since the function call 'QHostInfo::abortHostLookup ( info )' in ~NetGet() is not working, maybe this is also the issue its
usage in QNetworkAccessManager?
    
## iptables
iptables -D OUTPUT -d 8.8.8.8 -j DROP    
or
iptables -D OUTPUT -p udp -d 8.8.8.8 -j DROP
    
on the host running ./qt-QNetworkAccessManager-issues does NOT have the same effect than unplugging the cable!    
      guest % cat /etc/resolv.conf
      nameserver 8.8.8.8


      guest % iptables -D OUTPUT -p udp -d 8.8.8.8 -j DROP

      
      guest % ./qt-QNetworkAccessManager-issues
      make: Nothing to be done for `first'.
      NetGet about to do lookupHost 
      lookupHost ID =  1 
      NetGet now invoking quit() at QCoreApplication::instance() 
      ------------------------------------------------------------------------------------------
        type the letter 'q' and hit RETURN to run quit(
      ------------------------------------------------------------------------------------------
      --------------------------------------------
      printResults () 
      -------------------------------------------- 
      canceling lookupHost ID =  1 
      ~NetGet 

      
      guest % ping 8.8.8.8
      PING 8.8.8.8 (8.8.8.8) 56(84) bytes of data.
      ping: sendmsg: Operation not permitted
      ping: sendmsg: Operation not permitted
      ping: sendmsg: Operation not permitted
      ^C
      --- 8.8.8.8 ping statistics ---
      3 packets transmitted, 0 received, 100% packet loss, time 2017ms
   
NOTE: interestingly it makes the situation even better, since if you unplug the cable now it instantly terminates anyway!    
    
# discussion 

## what should have happened
since i want to shutdown the program ASAP it is not important to wait for the DNS query. 

## what happens instead
here two example traces using a modified qt 4.8.x library

### experiment qt-QNetworkAccessManager-issues

the qt-QNetworkAccessManager-issues example program only calling lookupHost(..), which is also done by QNetworkAccessManager. 

    root@schiejo-VirtualBox ...amples/qt-QNetworkAccessManager-issues (git)-[master] # make &&  ./qt-QNetworkAccessManager-issues
    /usr/local/Trolltech/Qt-4.8.5/bin/qmake -o Makefile qt-QNetworkAccessManager-issues.pro
    make: Nothing to be done for `first'.
    NetGet about to do lookupHost 
    NetGet starting -  "14:43:51.739" 
    lookupHost 1 
    QHostInfoLookupManager 1 
    isEnabled 1 
    get 1 
    get after QMutexLocker -  "14:43:51.739" 
    get 2 
    QHostInfo 1 
    ~QHostInfo 1 
    QHostInfoRunnable 1 
    scheduleLookup wasDeleted:  false 
    scheduleLookup after QMutexLocker -  "14:43:51.740" 
    work 1 
    work after QMutexLocker -  "14:43:51.740" 
    lookupHost ID =  1 
    NetGet now invoking quit() at QCoreApplication::instance() 
    The time is  "14:43:51.740" 

    run 1 
    run before wasAborted( call ) 
    wasAborted 1 
    wasAborted after QMutexLocker -  "14:43:51.741" 
    run after wasAborted( call ) 
    QHostInfo 1 
    isEnabled 1 
    get 1 
    ------------------------------------------------------------------------------------------
      type the letter 'q' and hit RETURN to run quit(
    ------------------------------------------------------------------------------------------
    
    get after QMutexLocker -  "14:43:51.741" 
    get 2 
    QHostInfo 1 
    operator= 1 
    ~QHostInfo 1 
    QHostInfo 1 
    fromName starting -  "14:43:51.741" 
    QHostInfoAgent::fromName(www.qt-project.org) looking up...
    fromName passed locker thingy -  "14:43:51.742" 
    setHostName 1 
    fromName IDN? -  "14:43:51.742" 
    fromName before getaddrinfo -  "14:43:51.742" 
    q
    you typed:  "q" 
    input handling exited, program should quit in less than a second! 
    myQuit "14:43:53.170" 
    ~NetGet 
    fromName after getaddrinfo -  "14:44:31.784" 
    fromName Q_ADDRCONFIG -  "14:44:31.784" 
    fromName result == 0? -  "14:44:31.784" 
    setError 1 
    setErrorString 1 
    error 1 
    errorString 1 
    QHostInfoAgent::fromName(): error #2 Host not found
    operator= 1 
    ~QHostInfo 1 
    put 1 
    error 1 
    run before wasAborted( call ) 
    wasAborted 1 
    wasAborted after QMutexLocker -  "14:44:31.784" 
    run after wasAborted( call ) 
    setLookupId 1 
    run after QMutexLocker -  "14:44:31.784" 
    lookupFinished 1 
    lookupFinished after QMutexLocker -  "14:44:31.785" 
    work 1 
    work after QMutexLocker -  "14:44:31.785" 
    ~QHostInfo 1 
    ~QHostInfoLookupManager 1 
    ~QHostInfoLookupManager 
    ~QHostInfoLookupManager after calling clear 


    
    
    
## qt-http-206-example (which uses QNetworkAccessManager)

to prove that QNetworkAccessManager's problem is in lookupHost, here is another trace.

    root@schiejo-VirtualBox ...x-fuse/qt-examples/qt-http-206-example (git)-[master] # ./qt-http-206-example
    lookupHost 1 
    QHostInfoLookupManager 1 
    isEnabled 1 
    get 1 
    get after QMutexLocker -  "15:05:09.697" 
    get 2 
    QHostInfo 1 
    ~QHostInfo 1 
    QHostInfoRunnable 1 
    scheduleLookup wasDeleted:  false 
    scheduleLookup after QMutexLocker -  "15:05:09.697" 
    work 1 
    work after QMutexLocker -  "15:05:09.697" 
    run 1 
    run before wasAborted( call ) 
    wasAborted 1 
    wasAborted after QMutexLocker -  "15:05:09.699" 
    run after wasAborted( call ) 
    QHostInfo 1 
    isEnabled 1 
    get 1 
    get after QMutexLocker -  "15:05:09.699" 
    get 2 
    QHostInfo 1 
    operator= 1 
    ~QHostInfo 1 
    QHostInfo 1 
    fromName starting -  "15:05:09.700" 
    QHostInfoAgent::fromName(www.qt-project.org) looking up...
    fromName passed locker thingy -  "15:05:09.700" 
    setHostName 1 
    fromName IDN? -  "15:05:09.701" 
    fromName before getaddrinfo -  "15:05:09.701" 
    'q', then RETURN or CTRL+C to exit immediately; 'c' then RETURN to cancel further downloads 
    >> this example will download something defined in NetGet.cpp (if network is working) << 
    type the letter 'q' and hit return if you want to cancel the download and exit instantly without waiting for the download to finish 
    q
    you typed:  "q" 
    input handling exited 
    NetGet::cancelDownload() called 
    'q', then RETURN or CTRL+C to exit immediately; 'c' then RETURN to cancel further downloads 
    ~NetGet 
    fromName after getaddrinfo -  "15:05:49.743" 
    fromName Q_ADDRCONFIG -  "15:05:49.743" 
    fromName result == 0? -  "15:05:49.743" 
    setError 1 
    setErrorString 1 
    error 1 
    errorString 1 
    QHostInfoAgent::fromName(): error #2 Host not found
    operator= 1 
    ~QHostInfo 1 
    put 1 
    error 1 
    run before wasAborted( call ) 
    wasAborted 1 
    wasAborted after QMutexLocker -  "15:05:49.744" 
    run after wasAborted( call ) 
    setLookupId 1 
    run after QMutexLocker -  "15:05:49.744" 
    lookupFinished 1 
    lookupFinished after QMutexLocker -  "15:05:49.744" 
    work 1 
    work after QMutexLocker -  "15:05:49.744" 
    ~QHostInfo 1 
    ~QHostInfoLookupManager 1 
    ~QHostInfoLookupManager 
    ~QHostInfoLookupManager after calling clear 


## possible solutions
the current implementation of QHostInfo::lookupHost is using my operating system's POSIX::getaddrinfo(..) function call, 
see qt-everywhere-opensource-src-4.8.5/src/network/kernel/qhostinfo.cpp in function: QHostInfoAgent::fromName(..)


the problem basically is:
- when shutting down my example program, QHostInfoLookupManager::~QHostInfoLookupManager() will call clear();
  clear() for example calls     
  
      threadPool.waitForDone();

- there are more such calls, but i didn't find them all, for example (maybe this is another one):

      QHostInfoLookupManager::QHostInfoLookupManager() : mutex(QMutex::Recursive), wasDeleted(false)
      {
          moveToThread(QCoreApplicationPrivate::mainThread());
          connect(QCoreApplication::instance(), SIGNAL(destroyed()), SLOT(waitForThreadPoolDone()), Qt::DirectConnection);
      }
    
### solution
either
* make the getaddrinfo(..) call interruptible like read(..) with EINTR (but getaddrinfo seems not to share this concept used in read/write syscalls)
or
* don't wait for the running QHostInfoAgent lookup on shutdown, this would mean to modify the clear() implementation and some other parts probably

the implemetation:
    info = QHostInfo::lookupHost("www.qt-project.org", this, SLOT(printResults(QHostInfo)));

    will create a QHostlookupInfoManager (singleton implementation) and add the runnalbe to a list of scheduled runnables with:
      runnable = QHostInfoRunnable(name,id);
      manager->scheduleLookup(runnable);
      
    later, when the runnabled is scheduled for running (in my example this happens in 1ms or something) it will call
    QHostInfoRunnable::run() and there it will call QHostInfoAgent::fromName(URL); 
    and occationally get stuck ;P
