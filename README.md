# related issues
someone else had the same discovery here:
 - https://qt-project.org/forums/viewthread/11763
 - http://www.qtcentre.org/threads/41649-QNetworkAccessManager-smart-handling-of-timeouts-network-errors

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
    
# possible solutions
- find out why the function call 'QHostInfo::abortHostLookup ( info )' in ~NetGet() is not working
- check if QNetworkAccessManager is affected by the same issue, if not
- consider using 'QHostInfo::abortHostLookup ( x )' for all running requests



