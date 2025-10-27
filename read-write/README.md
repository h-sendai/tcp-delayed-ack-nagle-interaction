# TCP Delayed Ack and Nagle algorithm Interaction

TCP delayed ackとNagleアルゴリズムの相互作用による
遅延の観測。

delayed ackのカウンタは/proc/net/netstatにある。
```
netstat -s -t | grep delayed
```
でいままで送信したdelayed ackの回数が取得できる
(netstatオプションの``-s``はstatstics, ``-t``はtcpの意味)。

クライアントが、headerとbodyをサーバーに送る。
サーバーではheaderとbodyのふたつの情報がきたらデータを
作ることができるようになり、作ったデータををreplyとして
返すシステムを模擬してみる。

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

まず単純にheaderとbodyを2回write()するプログラムを作る。

- クライアントはheaderとbodyをサーバーに送る。ここではそれぞれでwrite()する。
- サーバーはheaderとbodyを読んだらreplyを1回write()する

##

- PC1でserverプログラムを作動
- PC2でtcpdumpでキャプチャしつつclientを作動

させたときのclientのログ:

```
 1  0.000094 0.000000 client: will write header
 2  0.000145 0.000051 client: wrote header
 3  0.000149 0.000004 client: will write body
 4  0.000152 0.000003 client: wrote body
 5  0.000153 0.000001 client: will read reply
 6  0.000321 0.000168 client: got reply

 7  1.000384 1.000063 client: will write header
 8  1.000402 0.000018 client: wrote header
 9  1.000405 0.000003 client: will write body
10  1.000408 0.000003 client: wrote body
11  1.000410 0.000002 client: will read reply
12  1.040998 0.040588 client: got reply

13  2.041059 1.000061 client: will write header
14  2.041075 0.000016 client: wrote header
15  2.041078 0.000003 client: will write body
16  2.041080 0.000002 client: wrote body
17  2.041082 0.000002 client: will read reply
18  2.081995 0.040913 client: got reply

19  3.082056 1.000061 client: will write header
20  3.082071 0.000015 client: wrote header
21  3.082074 0.000003 client: will write body
22  3.082077 0.000003 client: wrote body
23  3.082078 0.000001 client: will read reply
24  3.122994 0.040916 client: got reply

25  4.123054 1.000060 client: will write header
26  4.123070 0.000016 client: wrote header
27  4.123073 0.000003 client: will write body
28  4.123076 0.000003 client: wrote body
29  4.123078 0.000002 client: will read reply
30  4.163992 0.040914 client: got reply

31  5.164052 1.000060 client: will write header
32  5.164067 0.000015 client: wrote header
33  5.164070 0.000003 client: will write body
34  5.164073 0.000003 client: wrote body
35  5.164074 0.000001 client: will read reply
36  5.204991 0.040917 client: got reply
```

最初のreplyはすぐに返ってきているが、2回目以降は40ミリ秒よけいに
時間がかかっている。

tcpdumpのログ[tcpdump.txt](tcpdump.txt)
をみると、2回目以降は11, 17行め, ...のようにサーバー側から
ackが返るのに40ミリ秒かかっていて、ackを受信後、2個目の
リクエストデータを送っていることが確認できる。

プログラムのログとパケットキャプチャをまぜたログ
[prog-packet.log.txt](prog-packet.log.txt)

server側で2回目以降から40m秒のdelayed ackになるというのが
気になる。``server -v``と``client -v``というオプションを付けて
TCP_QUICKACKが有効になっているかどうか調べて有効になっていたら
そのむね表示するようにしてみた。

- [サーバーのログ](quickack-state/server.log)
- [クライアントのログ](quickack-state/client.log)

コマンドラインでquickackの設定は指定していないが
サーバー側は最初の2回のread()でTCP_QUICKACKオプションが
有効化されていることがわかる。クライアント側はいつも
有効化されている。

## -D (nodelay), -q (quick ack)オプション

server, clientともに-D (TCP_NODELAY)オプション、-q (TCP_QUICkACK)オプション
をつけられるようにしてある。

このシステムの場合、40ミリ秒よけいに時間がかかるのは
server側で最初の512バイトヘッダを受信したあと、すぐにACKを返さず
delayed ackモードが使われ、40ミリ秒後にackが帰ることと
client側が最初のheaderを送信したあとNagleアルゴリズムを使って
server側からのackを受信するまでbodyを送信しないという
delayed ackとNagleアルゴリズムの相互作用が生じているからである。

このシステムの場合、40ミリ秒よけいに時間がかかるのを解消するには
1. クライアント側でheaderとbodyを書くのにwritev()を使う（この上のディレクトリのwritev)
2. サーバー側でheaderを読んだら即座にackを返すためにquickackを使う(クライアント側はオプションなしに./client remote_hostで起動、サーバ側は./server -qでquickackを使うように起動する(サーバ側でheaderを読んだあとすぐにackを返すようになるので、クライアント側でNagleモードにはいっているのがackの受信で解消され、すぐにbodyを送ることができるようになる)
3. サーバー側はオプションなしに起動(./server)し、クライアント側はTCP_NODELAYを指定する(./client -D remote_host) (headerとbodyが連続して送信され、サーバー側はheaderとbodyがきたのでリプライを返すことができるようになる)

### ip route quickack

ip routeコマンドを使うと、ルート毎にquickackを設定することができる。
たとえば192.168.10.200のIPアドレスが付いているNIC enp6s0が
あったとしてip route changeコマンドを実行する。

```
# ip route show
(略)
192.168.10.0/24 dev enp6s0 proto kernel scope link src 192.168.10.200 metric 100 

# ip route change 192.168.10.0/24 dev enp6s0 proto kernel scope link src 192.168.10.200 metric 100 quickack 1

# ip route show
(略)
192.168.10.0/24 dev enp6s0 proto kernel scope link src 192.168.10.200 metric 100 quickack 1
```

これで192.168.10.0/24からのパケットはTCP_QUICKACKオプションを付けることなしに
quicack付きで読むことになる。
この設定をしたPCでserverを動かし、他PCでclientを動かしたときのパケットキャプチャのログ:
[ログ](prog-net-log/packet-log.quickack-enabled-on-route)。
この設定なしの場合はdelayed ackが発動し、headerパケットが到着後、
40ミリ秒後にackがでるが、この設定をするとheaderパケット到着後、
すぐにackを出していることがわかる。

## Delayed Ackを有効化するトリガー ???

上の例をみると1回めのheader, body, replyの受け取りはディレイなしに
終了していて、2回めのheaderを送ったときにサーバー側でdelayed ackが
発動している。この違いは何だろうかとおもって他プログラムで
「クライアントが サーバー側に接続したら1秒に1回、512バイトのデータを
送る（クライアントはふつうに512バイトを読む）」という
システムを試してみた。クライアントはackをdelayさせることなく応答
していた。

考えてみると、サーバーはheaderとbodyを受信したあと、replyを
クライアントに返している。Delayed ackはackをpiggybackできる
データがあればペイロードを送るついでにackを送る、というもの
なのでこれが原因かもしれない。
確認用にclient, serverともに-Nというオプションをつけて
client側はheader, bodyを送るのみ、server側はheader, bodyを
読むのみでreplyを返さないというシナリオで動作させてみたら、
serverがdelayed ackでackを返すということはなかった。

データの流れが一方方向であるならdelayed ackの発動条件の
ひとつは成立しないということだろうか（送られてきたデータを
あまり読まないのでackを出す間隔が短くなるということはある。）

ここではheader, bodyと2個のデータを送るとdelayed ackが
起こることをみたが、

```
クライアント                                 サーバー

header_byte_size (512) バイト ---->
                              <---- reply_byte_size (1448) バイト

(スリープusleep_sec秒)
header_byte_size (512) バイト ---->
                              <---- reply_byte_size (1448) バイト
```

のようにクライアントから送るパケットが1個だとdelayed ackは
起こらない。

## データサイズをMSSにする

上の例ではheader,body,replyともにMSS未満のものだったが
1448バイト (MSS 1500 - Timestampオプション12バイト)にすると
delayed ackは生じない(1448バイトなのでNagleアルゴリズムで
遅くなることはない)。

サーバー:
```
./server -H 1448 -B 1448 -R 1448
```
クライアント:
```
./client -H 1448 -B 1448 -R 1448 サーバーIPアドレス
```
[1448バイト時のプログラムログ+キャプチャログ](prog-net-log/1448.log)
