# COMP20023-DNS-Server
Write a dummy DNS server, as the assignment of the Computer System subject

# 1. Background
Note: This project will contain some reading of standards as well as writing code. If you ever write an implementation of a protocol, you will need to read standards such as these. Therefore, becoming familiar with the format and terminology is an important part of the ﬁeld of computer systems. You will be pointed to the relevant sections so that you do not spend your whole time reading the more arcane parts of the text.

The Domain Name System (DNS) provides, among other things, the mapping between human-meaningful hostnames like lms.unimelb.edu.au and the numeric IP addresses that indicate where packets should be sent. DNS consists of a hierarchy of servers, each knowing a portion of the complete mapping.

In this project, you will write a DNS server that accepts requests for IPv6 addresses and serves them either from its own cache or by querying servers higher up the hierarchy. Each transaction consists of at most four messages: one from your client to you, one from you to your upstream server, one from your upstream server to you and one from you to your client. The middle two can be sometimes skipped if you cache some of the answers.

The format for DNS request and response messages are described in [1].

In a DNS system, the entry mapping a name to an IPv6 address is called a AAAA (or “quad A”) record [2]. Its “record type” is 28 (QType in [2]).

The server will also keep a log of its activities. This is important for reasons such as detecting denial-of-service attacks, as well as allowing service upgrades to reﬂect usage patterns.

For the log, you will need to print a text version of the IPv6 addresses. IPv6 addresses are 128 bits long. They are represented in text as eight colon-separated strings of 16-bit numbers expressed in hexadecimal. As a shorthand, a string of consecutive 16-bit numbers that are all zero may be replaced by a single “::”. Details are in [3].

# 2. Specification
Task: Write a miniature DNS server that will serve AAAA queries.

This project has three variants, with increasing levels of diﬃculty. It is not expected that most students will complete all tasks; the hard ones are to challenge those few who want to be challenged.

It is expected that about half the class will complete the Standard option (and do it well – you can get an H1 by getting full marks on this), a third to complete the Cache option, and a sixth to complete the Non-blocking option. If you think the project is taking too much time, make sure you are doing the Standard option.

You should create a Makeﬁle that produces an executable named ’dns_svr’.

## 2.1 Standard
Accept a DNS “AAAA” query over TCP on port 8053. Forward it to a server whose IPv4 address is the ﬁrst command-line argument and whose port is the second command-line argument. (For testing, use the value in /etc/resolv.conf on your server and port 53). Send the response back to the client who sent the request, over the same TCP connection. There will be a separate TCP connection for each query/response with the client. Log these events, as described below.

Note that DNS usually uses UDP, but this project will use TCP because it is a more useful skill for you to learn. A DNS message over TCP is slightly diﬀerent from that over UDP: it has a two-byte header that specify the length (in bytes) of the message, not including the two-byte header itself [4, 5]. This means that you know the size of the message before you read it, and can malloc() enough space for it.

Assume that there is only one question in the DNS request you receive, although the standard allows there to be more than one. If there is more than one answer in the reply, then only log the ﬁrst one, but always reply to the client with the entire list of answers. If there is no answer in the reply, log the request line only. If the ﬁrst answer in the response is not a AAAA ﬁeld, then do not print a log entry (for any answer in the response).

The program should be ready to accept another query as soon as it has processed the previous query and response. (If Non-blocking option is implemented, it must be ready before this too.)

Your server should not shut down by itself. SIGINT (like CTRL-C) will be used to terminate your server between test cases.
