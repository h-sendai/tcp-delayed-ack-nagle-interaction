# TCP Delayed Ack and Nagle algorithm Interaction

TCP delayed ackとNagleアルゴリズムの相互作用による
遅延の観測。

```
クライアント                                 サーバー

header_byte_size (512) バイト ---->
body_byte_size   (512) バイト ---->
                              <---- reply_byte_size (1448) バイト

(スリープusleep_sec秒)
header_byte_size (512) バイト ---->
body_byte_size   (512) バイト ---->
                              <---- reply_byte_size (1448) バイト
```

- クライアントはheaderとbodyをサーバーに送る。ここではそれぞれでwrite()する。
- サーバーはheaderとbodyを読んだらreplyを1回write()する

##

- PC1でserverプログラムを作動
- PC2でtcpdumpでキャプチャしつつclientを作動

させたときのclientのログ:

```
 1  0.000000 0.000000 client: wrote 1st request data
 2  0.000047 0.000047 client: wrote 2nd data
 3  0.000197 0.000150 client: got reply

 4  1.000271 1.000074 client: wrote 1st request data
 5  1.000280 0.000009 client: wrote 2nd data
 6  1.040374 0.040094 client: got reply

 7  2.040446 1.000072 client: wrote 1st request data
 8  2.040456 0.000010 client: wrote 2nd data
 9  2.081366 0.040910 client: got reply

10  3.081440 1.000074 client: wrote 1st request data
11  3.081449 0.000009 client: wrote 2nd data
12  3.122373 0.040924 client: got reply

13  4.122444 1.000071 client: wrote 1st request data
14  4.122450 0.000006 client: wrote 2nd data
15  4.163376 0.040926 client: got reply
```

最初のreplyはすぐに返ってきているが、2回目以降は40ミリ秒よけいに
時間がかかっている。

tcpdumpのログ[tcpdump.txt](tcpdump.txt)
をみると、2回目以降は11, 17行め, ...のようにサーバー側から
ackが返るのに40ミリ秒かかっていて、ackを受信後、2個目の
リクエストデータを送っていることが確認できる。
