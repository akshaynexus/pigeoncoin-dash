Dash Core version 0.16
======================

Release is now available from:

  <https://www.dash.org/downloads/#wallets>

This is a new major version release, bringing new features, various bugfixes
and other improvements.

This release is mandatory for all nodes.

Please report bugs using the issue tracker at github:

  <https://github.com/dashpay/dash/issues>


Upgrading and downgrading
=========================

How to Upgrade
--------------

If you are running an older version, shut it down. Wait until it has completely
shut down (which might take a few minutes for older versions), then run the
installer (on Windows) or just copy over /Applications/Dash-Qt (on Mac) or
pigeond/pigeon-qt (on Linux). If you upgrade after DIP0003 activation and you were
using version < 0.13 you will have to reindex (start with -reindex-chainstate
or -reindex) to make sure your wallet has all the new data synced. Upgrading
from version 0.13 should not require any additional actions.

When upgrading from a version prior to 0.14.0.3, the
first startup of Dash Core will run a migration process which can take a few
minutes to finish. After the migration, a downgrade to an older version is only
possible with a reindex (or reindex-chainstate).

Downgrade warning
-----------------

### Downgrade to a version < 0.14.0.3

Downgrading to a version older than 0.14.0.3 is no longer supported due to
changes in the "evodb" database format. If you need to use an older version,
you must either reindex or re-sync the whole chain.

### Downgrade of masternodes to < 0.16

Starting with this release, masternodes will verify the protocol version of other
masternodes. This will result in PoSe punishment/banning for outdated masternodes,
so downgrading is not recommended.

Notable changes
===============

Block Reward Reallocation
-------------------------
This version implements Block Reward Reallocation which was proposed in order
to slow the growth rate of Dash’s circulating supply by encouraging the
formation of masternodes and was voted in by the network. The resulting allocation
will split all non-proposal block rewards 40% toward miners and 60% toward
masternodes in the end-state once the transition period is complete.

The reallocation will take place over 4.5 years with a total of 18 reallocation
periods between the start and end state. The transition is being made gradually
to avoid market volatility and minimize network disruption.

Note that this is a hardfork which must be activated by miners. To do this they
should start creating blocks signalling bit 5 in the `version` field of the block header.

### Reallocation periods

Each reallocation period will last three superblock cycles (approximately one
quarter). The following table shows the split of the non-proposal block rewards during
each period.

|Event|Miner|Masternode|
|--|-----|-----|
|Now|50.0%|50.0%|
|1 |48.7%|51.3%|
|2 |47.4%|52.6%|
|3 |46.7%|53.3%|
|4 |46.0%|54.0%|
|5 |45.4%|54.6%|
|6 |44.8%|55.2%|
|7 |44.3%|55.7%|
|8 |43.8%|56.2%|
|9 |43.3%|56.7%|
|10|42.8%|57.2%|
|11|42.3%|57.7%|
|12|41.8%|58.2%|
|13|41.5%|58.5%|
|14|41.2%|58.8%|
|15|40.9%|59.1%|
|16|40.6%|59.4%|
|17|40.3%|59.7%|
|18|40.1%|59.9%|
|End|40.0%|60.0%|

Dynamic Activation Thresholds
-----------------------------
In Dash we have used lower thresholds (80% vs 95% in BTC) to activate upgrades
via a BIP9-like mechanism for quite some time. While it's preferable to have as much
of the network hashrate signal update readiness as possible, this can result in
quite lengthy upgrades if one large non-upgraded entity stalls
all progress. Simply lowering thresholds even further can result in network
upgrades occurring too quickly and potentially introducing network instability. This version
implements BIP9-like dynamic activation thresholds which drop from some initial
level to a minimally acceptable one over time at an increasing rate. This provides
a safe non-blocking way of activating proposals.

This mechanism applies to the Block Reward Reallocation proposal mentioned above.
Its initial threshold is 80% and it will decrease to a minimum of 60% over the
course of 10 periods. Each period is 4032 blocks (approximately one week).

Concentrated Recovery
---------------------
In the current system, signature shares are propagated to all LLMQ members
until one of them has collected enough shares to recover the signature. All
members keep propagating and verifying each share until this recovered signature
is propagated in the LLMQ. This causes significant load on the LLMQ and results
in decreased throughput.

This new system initially sends all shares to a single deterministically selected node,
so that this node can recover the signature and propagate the recovered signature.
This way only the recovered signature needs to be propagated and verified by all
members. After sending their share to this node, each member waits for a
timeout and then sends their share to another deterministically selected member.
This process is repeated until a recovered signature is finally created and propagated.

This timeout begins at two seconds and increases exponentially up to ten seconds
(i.e. `2,4,8,10,10`) for each node that times out. This is to minimize the time
taken to generate a signature if the recovery node is down, while also
minimizing the traffic generated when the network is under stress.

The new system is activated with the newly added `SPORK_21_QUORUM_ALL_CONNECTED`
which has two hardcoded values with a special meaning: `0` activates Concentrated
Recovery for every LLMQ and `1` excludes `400_60` and `400_85` quorums.

Increased number of masternode connections
------------------------------------------
To implement "Concentrated Recovery", it is now necessary for all members of a LLMQ
to connect to all other members of the same LLMQ. This significantly increases the general
connection count for masternodes. Although these intra-quorum connections are less resource
intensive than normal p2p connections (as they only exchange LLMQ/masternode related
messages), masternode hardware and network requirements will still be higher than before.

Initially this change will only be activated for the smaller LLMQs (50 members).
Eventually it may be activated for larger quorums (400 members).

This is also controlled via `SPORK_21_QUORUM_ALL_CONNECTED`.

Masternode Connection Probing
-----------------------------
While each LLMQ member must have a connection to each other member, it's not necessary
for all members to actually connect to all other members. This is because only
one of a pair of two masternodes need to initiate the connection while the other one can
wait for an incoming connection. Probing is done in the case where a masternode doesn't
really need an outbound connection, but still wants to verify that the other side
has its port open. This is done by initiating a short lived connection, waiting
for `MNAUTH` to succeed and then disconnecting again.

After this process, each member of a LLMQ knows which members are unable to accept
connections, after which they will vote on these members to be "bad".

Masternode Minimum Protocol Version Checks
------------------------------------------
Members of LLMQs will now also check all other members for minimum protocol versions
while in DKG. If a masternode determines that another LLMQ member has a protocol
version that is too low, it will vote for the other member to be "bad".

PoSe punishment/banning
-----------------------
If 80% of all LLMQ members voted for the same member to be bad, it is excluded
in the final stages of the DKG. This causes the bad masternode to get PoSe punished
and then eventually PoSe banned.

Network performance improvements
--------------------------------
This version of Dash Core includes multiple optimizations to the network and p2p message
handling code. The most important one is the introduction of `epoll` on linux-based
systems. This removes most of the CPU overhead caused by the sub-optimal use of `select`,
which could easily use up 50-80% of the CPU time spent in the network thread when many
connections were involved.

Since these optimizations are exclusive to linux, it is possible that masternodes hosted
on windows servers will be unable to handle the network load and should consider migrating
to a linux based operating system.

Other improvements were made to the p2p message handling code, so that LLMQ
related connections do less work than full/normal p2p connections.

Wallet files
------------
The `--wallet=<path>` option now accepts full paths instead of requiring
wallets to be located in the `-walletdir` directory.

If `--wallet=<path>` is specified with a path that does not exist, it will now
create a wallet directory at the specified location (containing a `wallet.dat`
data file, a `db.log` file, and `database/log.??????????` files) instead of just
creating a data file at the path and storing log files in the parent
directory. This should make backing up wallets more straightforward than
before because the specified wallet path can just be directly archived without
having to look in the parent directory for transaction log files.

For backwards compatibility, wallet paths that are names of existing data files
in the `--walletdir` directory will continue to be accepted and interpreted the
same as before.

When Dash Core is not started with any `--wallet=<path>` options, the name of
the default wallet returned by `getwalletinfo` and `listwallets` RPCs is
now the empty string `""` instead of `"wallet.dat"`. If Dash Core is started
with any `--wallet=<path>` options, there is no change in behavior, and the
name of any wallet is just its `<path>` string.

PrivateSend coin management improvements
----------------------------------------
A new algorithm for the creation of mixing denominations was implemented which
should reduce the number of the smallest inputs created and give users more
control on soft and hard caps. A much more accurate fee management algorithm was
also implemented which should fix issues for users who have custom fees
specified in wallet config or in times when network is under load.

There is a new "PrivateSend" tab in the GUI which allows spending fully
mixed coins only. The CoinControl feature was also improved to display coins
based on the tab it was opened in, to protect users from accidentally spending
mixed and non-mixed coins in the same transaction and to give better overview of
spendable mixed coins.

PrivateSend Random Round Mixing
-------------------------------
Some potential attacks on PrivateSend assume that all inputs had been mixed
for the same number of rounds (up to 16). While this assumption alone is not
enough to break PrivateSend privacy, it could still provide some additional
information for other methods like cluster analysis and help to narrow results.

With Random Round Mixing, an input will be mixed to N rounds like prior. However,
at this point, a salted hash of each input is used to pick ~50% of inputs to
be mixed for another round. This rule is then executed again on the new inputs.
This results in an exponential decay where if you mix a set
of inputs, half of those inputs will be mixed for N rounds, 1/4 will be mixed N+1,
1/8 will be mixed N+2, etc. Current implementation caps it at N+3 which results
in mixing an average of N+0.875 rounds and sounds like a reasonable compromise
given the privacy gains.

GUI changes
-----------
All dialogs, tabs, icons, colors and interface elements were reworked to improve
user experience, let the wallet look more consistent and to make the GUI more
flexible. There is a new "Appearance setup" dialog that will show up on the first start
of this version and a corresponding tab has been added to the options which allows the
user to pick a theme and to tweak the font family, the font weight and the font size.
This feature specifically should help users who had font size/scaling issues previously.

For advanced users and developers there is a new way to control the wallet's look
by specifying a path to custom css files via `--custom-css-dir`. Additionally, the new
`--debug-ui` will force Dash Core to reload the custom css files as soon as they get updated
which makes it possible to see and debug all css adjustments live in the running GUI.

From now on the "Pay To" field in "Send" and "PrivateSend" tabs also accepts Dash URIs.
The Dash address and the amount from the URI are assigned to corresponding fields automatically
if a Dash URI gets pasted into the field.

Sporks
------
Two new sporks were introduced in this version:
- `SPORK_21_QUORUM_ALL_CONNECTED` allows to control Concentrated Recovery and
Masternode Probing activation;
- `SPORK_22_PS_MORE_PARTICIPANTS` raises the number of users that can participate
in a single PrivateSend mixing transaction to 20.

Sporks `SPORK_15_DETERMINISTIC_MNS_ENABLED`, `SPORK_16_INSTANTSEND_AUTOLOCKS`
and `SPORK_20_INSTANTSEND_LLMQ_BASED` which were previously deprecated in v0.15
are completely removed now. `SPORK_6_NEW_SIGS` was never activated on mainnet
and was also removed in this version.

Build system
------------
The minimum version of the GCC compiler required to compile Dash Core is now 4.8.
The minimum version of Qt is now 5.5.1. Some packages in `depends/` as well as
`secp256k1` and `leveldb` subtrees were updated to newer versions.

RPC changes
-----------
There are a few changes in existing RPC interfaces in this release:
- `getpeerinfo` has new field `masternode` to indicate whether connection was
  due to masternode connection attempt
- `getprivatesendinfo` `denoms` field was replaced by `denoms_goal` and
  `denoms_hardcap`
- `listunspent` has new filter option `coinType` to be able to filter different
  types of coins (all, mixed etc.)
- `protx diff` returns more detailed information about new quorums
- `quorum dkgstatus` shows `quorumConnections` for each LLMQ with detailed
  information about each participating masternode
- `quorum sign` has an optional `quorumHash` argument to pick the exact quorum
- `socketevents` in `getnetworkinfo` rpc shows the socket events mode,
  either `epoll`, `poll` or `select`

There are also new RPC commands:
- `quorum selectquorum` returns the quorum that would/should sign a request

There are also new RPC commands backported from Bitcoin Core 0.16:
- `help-console` for more info about using console in Qt
- `loadwallet` loads a wallet from a wallet file or directory
- `rescanblockchain` rescans the local blockchain for wallet related transactions
- `savemempool` dumps the mempool to disk

Please check Bitcoin Core 0.16 release notes in a [section](#backports-from-bitcoin-core-016)
below and `help <command>` in rpc for more information.

Command-line options
--------------------
Changes in existing cmd-line options:

New cmd-line options:
- `--custom-css-dir`
- `--debug-ui`
- `--disablegovernance`
- `--font-family`
- `--font-scale`
- `--font-weight-bold`
- `--font-weight-normal`
- `--llmqdevnetparams`
- `--llmqtestparams`
- `--privatesenddenomsgoal`
- `--privatesenddenomshardcap`
- `--socketevents`

Few cmd-line options are no longer supported:
- `--litemode`
- `--privatesenddenoms`

There are also new command-line options backported from Bitcoin Core 0.16:
- `--addrmantest`
- `--debuglogfile`
- `--deprecatedrpc`
- `--enablebip61`
- `--getinfo`
- `--stdinrpcpass`

Please check Bitcoin Core 0.16 release notes in a [section](#backports-from-bitcoin-core-016)
below and `Help -> Command-line options` in Qt wallet or `dashd --help` for more information.

Backports from Bitcoin Core 0.16
--------------------------------

Most of the changes between Bitcoin Core 0.15 and Bitcoin Core 0.16 have been
backported into Dash Core 0.16. We only excluded backports which do not align
with Dash, like SegWit or RBF related changes.

You can read about changes brought by backporting from Bitcoin Core 0.16 in
following docs:
- https://github.com/bitcoin/bitcoin/blob/master/doc/release-notes/release-notes-0.16.0.md
- https://github.com/bitcoin/bitcoin/blob/master/doc/release-notes/release-notes-0.16.1.md
- https://github.com/bitcoin/bitcoin/blob/master/doc/release-notes/release-notes-0.16.2.md

Some other individual PRs were backported from versions 0.17+, you can find the
full list of backported PRs and additional fixes in [release-notes-0.16-backports.md](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.16-backports.md)

Miscellaneous
-------------
A lot of refactoring, code cleanups and other small fixes were done in this release.

0.16 Change log
===============

See detailed [set of changes](https://github.com/dashpay/dash/compare/v0.14.0.5...dashpay:v0.15.0.0).

- [`3c055bf79`](https://github.com/dashpay/dash/commit/3c055bf79) Bump nMinimumChainWork and defaultAssumeValid (#3336)
- [`818e7a6f7`](https://github.com/dashpay/dash/commit/818e7a6f7) Update release notes
- [`9d5c3d12e`](https://github.com/dashpay/dash/commit/9d5c3d12e) Try to actually accept newly created dstx-es into masternode's mempool (#3332)
- [`f23e722da`](https://github.com/dashpay/dash/commit/f23e722da) Switch CLIENT_VERSION_IS_RELEASE to `true` for v0.15 (#3306)
- [`b57f1dac8`](https://github.com/dashpay/dash/commit/b57f1dac8) Update release notes
- [`15c6df583`](https://github.com/dashpay/dash/commit/15c6df583) Bring back "about" menu icon (#3329)
- [`2c30818f7`](https://github.com/dashpay/dash/commit/2c30818f7) Add pubKeyOperator to `quorum info` rpc response (#3327)
- [`2bbf78c1b`](https://github.com/dashpay/dash/commit/2bbf78c1b) Update release-notes.md
- [`2c305d02d`](https://github.com/dashpay/dash/commit/2c305d02d) Update translations 2020-02-03 (#3322)
- [`672e18e48`](https://github.com/dashpay/dash/commit/672e18e48) Only sync mempool from v0.15+ (proto 70216+) nodes (#3321)
- [`829bde81e`](https://github.com/dashpay/dash/commit/829bde81e) Fix dark text on dark background in combobox dropdowns on windows (#3315)
- [`c0a671e84`](https://github.com/dashpay/dash/commit/c0a671e84) Fix node protection logic false positives (#3314)
- [`8d5fc6e0a`](https://github.com/dashpay/dash/commit/8d5fc6e0a) Merge #13162: [net] Don't incorrectly log that REJECT messages are unknown.
- [`9e711befd`](https://github.com/dashpay/dash/commit/9e711befd) More of 13946
- [`e5e3572e9`](https://github.com/dashpay/dash/commit/e5e3572e9) Merge #13946: p2p: Clarify control flow in ProcessMessage
- [`dbbc51121`](https://github.com/dashpay/dash/commit/dbbc51121) Add `automake` package to dash-win-signer's packages list (#3307)
- [`fd0f24335`](https://github.com/dashpay/dash/commit/fd0f24335) [Trivial] Release note update (#3308)
- [`058872d4f`](https://github.com/dashpay/dash/commit/058872d4f) Update release-notes.md
- [`546e69f1a`](https://github.com/dashpay/dash/commit/546e69f1a) Fix CActiveMasternodeManager::GetLocalAddress to prefer IPv4 if multiple local addresses are known (#3304)
- [`e4ef7e8d0`](https://github.com/dashpay/dash/commit/e4ef7e8d0) Drop unused `invSet` in `CDKGSession` (#3303)
- [`da7686c93`](https://github.com/dashpay/dash/commit/da7686c93) Update translations 2020-01-23 (#3302)
- [`6b5d3edae`](https://github.com/dashpay/dash/commit/6b5d3edae) Fix dip4-coinbasemerkleroots.py race condition (#3297)
- [`a8213cadb`](https://github.com/dashpay/dash/commit/a8213cadb) Various fixes for DSTX-es (#3295)
- [`2c26bdf2d`](https://github.com/dashpay/dash/commit/2c26bdf2d) Update release-notes.md
- [`1d9adbe63`](https://github.com/dashpay/dash/commit/1d9adbe63) Replace generic CScopedDBTransaction with specialized CEvoDBScopedCommitter (#3292)
- [`8fd486c6b`](https://github.com/dashpay/dash/commit/8fd486c6b) Translations 2020-01 (#3192)
- [`3c54f6527`](https://github.com/dashpay/dash/commit/3c54f6527) Bump copyright year to 2020 (#3290)
- [`35d28c748`](https://github.com/dashpay/dash/commit/35d28c748) Update man pages (#3291)
- [`203cc9077`](https://github.com/dashpay/dash/commit/203cc9077)  trivial: adding SVG and high contrast icons  (#3209)
- [`e875d4925`](https://github.com/dashpay/dash/commit/e875d4925) Define defaultTheme and darkThemePrefix as constants and use them instead of plain strings (#3288)
- [`1d203b422`](https://github.com/dashpay/dash/commit/1d203b422) Bump PROTOCOL_VERSION to 70216 (#3287)
- [`b84482ac5`](https://github.com/dashpay/dash/commit/b84482ac5) Let regtest have its own qt settings (#3286)
- [`1c885bbed`](https://github.com/dashpay/dash/commit/1c885bbed) Only load valid themes, fallback to "Light" theme otherwise (#3285)
- [`ce924278d`](https://github.com/dashpay/dash/commit/ce924278d) Don't load caches when blocks/chainstate was deleted and also delete old caches (#3280)
- [`ebf529e8a`](https://github.com/dashpay/dash/commit/ebf529e8a) Drop new connection instead of old one when duplicate MNAUTH is received (#3272)
- [`817cd9a17`](https://github.com/dashpay/dash/commit/817cd9a17) AppInitMain should quit early and return `false` if shutdown was requested at some point (#3267)
- [`42e104932`](https://github.com/dashpay/dash/commit/42e104932) Tweak few more strings re mixing and balances (#3265)
- [`d30eeb6f8`](https://github.com/dashpay/dash/commit/d30eeb6f8) Use -Wno-psabi for arm builds on Travis/Gitlab (#3264)
- [`1df3c67a8`](https://github.com/dashpay/dash/commit/1df3c67a8) A few fixes for integration tests (#3263)
- [`6e50a7b2a`](https://github.com/dashpay/dash/commit/6e50a7b2a) Fix params.size() check in "protx list wallet" RPC (#3259)
- [`1a1cec224`](https://github.com/dashpay/dash/commit/1a1cec224) Fix pull request detection in .gitlab-ci.yml (#3256)
- [`31afa9c0f`](https://github.com/dashpay/dash/commit/31afa9c0f) Don't disconnect masternode connections when we have less then the desired amount of outbound nodes (#3255)
- [`cecbbab3c`](https://github.com/dashpay/dash/commit/cecbbab3c) move privatesend rpc methods from rpc/masternode.cpp to new rpc/privatesend.cpp (#3253)
- [`8e054f374`](https://github.com/dashpay/dash/commit/8e054f374) Sync mempool from other nodes on start (#3251)
- [`474f25b8d`](https://github.com/dashpay/dash/commit/474f25b8d) Push islock invs when syncing mempool (#3250)
- [`3b0f8ff8b`](https://github.com/dashpay/dash/commit/3b0f8ff8b) Skip mnsync restrictions for whitelisted and manually added nodes (#3249)
- [`fd94e9c38`](https://github.com/dashpay/dash/commit/fd94e9c38) Streamline, refactor and unify PS checks for mixing entries and final txes (#3246)
- [`c42b20097`](https://github.com/dashpay/dash/commit/c42b20097) Try to avoid being marked as a bad quorum member when we sleep for too long in SleepBeforePhase (#3245)
- [`db6ea1de8`](https://github.com/dashpay/dash/commit/db6ea1de8) Fix log output in CDKGPendingMessages::PushPendingMessage (#3244)
- [`416d85b29`](https://github.com/dashpay/dash/commit/416d85b29) Tolerate parent cache with empty cache-artifact directory (#3240)
- [`0c9c27c6f`](https://github.com/dashpay/dash/commit/0c9c27c6f) Add ccache to gitian packages lists (#3237)
- [`65206833e`](https://github.com/dashpay/dash/commit/65206833e) Fix menu bar text color in Dark theme (#3236)
- [`7331d4edd`](https://github.com/dashpay/dash/commit/7331d4edd) Bump wait_for_chainlocked_block_all_nodes timeout in llmq-is-retroactive.py to 30 sec when mining lots of blocks at once (#3238)
- [`dad102669`](https://github.com/dashpay/dash/commit/dad102669) Update static and dns seeds for mainnet and testnet (#3234)
- [`9378c271b`](https://github.com/dashpay/dash/commit/9378c271b) Modify makesseeds.py to work with "protx list valid 1" instead of "masternode list (#3235)
- [`91a996e32`](https://github.com/dashpay/dash/commit/91a996e32) Make sure mempool txes are properly processed by CChainLocksHandler despite node restarts (#3226)
- [`2b587f0eb`](https://github.com/dashpay/dash/commit/2b587f0eb) Slightly refactor CDKGSessionHandler::SleepBeforePhase (#3224)
- [`fdb05860e`](https://github.com/dashpay/dash/commit/fdb05860e) Don't join thread in CQuorum::~CQuorum when called from within the thread (#3223)
- [`4c00d98ea`](https://github.com/dashpay/dash/commit/4c00d98ea) Allow re-signing of IS locks when performing retroactive signing (#3219)
- [`b4b9d3467`](https://github.com/dashpay/dash/commit/b4b9d3467) Tests: Fix the way nodes are connected to each other in setup_network/start_masternodes (#3221)
- [`dfe99c950`](https://github.com/dashpay/dash/commit/dfe99c950) Decouple cs_mnauth/cs_main (#3220)
- [`08f447af9`](https://github.com/dashpay/dash/commit/08f447af9) Tests: Allow specifying different cmd-line params for each masternode (#3222)
- [`9dad60386`](https://github.com/dashpay/dash/commit/9dad60386) Tweak "Send" popup and refactor related code a bit (#3218)
- [`bb7a32d2e`](https://github.com/dashpay/dash/commit/bb7a32d2e) Add Dark theme (#3216)
- [`05ac4dbb4`](https://github.com/dashpay/dash/commit/05ac4dbb4) Dashify few strings (#3214)
- [`482a549a2`](https://github.com/dashpay/dash/commit/482a549a2) Add collateral, owner and voting addresses to masternode list table (#3207)
- [`37f96f5a3`](https://github.com/dashpay/dash/commit/37f96f5a3) Bump version to 0.15 and update few const-s/chainparams (#3204)
- [`9de994988`](https://github.com/dashpay/dash/commit/9de994988) Compliance changes to terminology (#3211)
- [`d475f17bc`](https://github.com/dashpay/dash/commit/d475f17bc) Fix styles for progress dialogs, shutdown window and text selection (#3212)
- [`df372ec5f`](https://github.com/dashpay/dash/commit/df372ec5f) Fix off-by-one error for coinbase txes confirmation icons (#3206)
- [`1e94e3333`](https://github.com/dashpay/dash/commit/1e94e3333) Fix styling for disabled buttons (#3205)
- [`7677b5578`](https://github.com/dashpay/dash/commit/7677b5578) Actually apply CSS styling to RPC console (#3201)
- [`63cc22d5e`](https://github.com/dashpay/dash/commit/63cc22d5e) More Qt tweaks (#3200)
- [`7aa9c43f8`](https://github.com/dashpay/dash/commit/7aa9c43f8) Few Qt tweaks (#3199)
- [`fd50c1c71`](https://github.com/dashpay/dash/commit/fd50c1c71) Hold cs_main/cs_wallet in main MakeCollateralAmounts (#3197)
- [`460e0f475`](https://github.com/dashpay/dash/commit/460e0f475) Fix locking of funds for mixing (#3194)
- [`415b81e41`](https://github.com/dashpay/dash/commit/415b81e41) Refactor some pow functions (#3198)
- [`b2fed3862`](https://github.com/dashpay/dash/commit/b2fed3862) A few trivial fixes for RPCs (#3196)
- [`f8296364a`](https://github.com/dashpay/dash/commit/f8296364a) Two trivial fixes for logs (#3195)
- [`d5cc88f00`](https://github.com/dashpay/dash/commit/d5cc88f00) Should mark tx as a PS one regardless of change calculations in CreateTransaction (#3193)
- [`e9235b9bb`](https://github.com/dashpay/dash/commit/e9235b9bb) trivial: Rename txid paramater for gobject voteraw (#3191)
- [`70b320bab`](https://github.com/dashpay/dash/commit/70b320bab) Detect masternode mode from masternodeblsprivkey arg (#3188)
- [`1091ab3c6`](https://github.com/dashpay/dash/commit/1091ab3c6) Translations201909 (#3107)
- [`251fb5e69`](https://github.com/dashpay/dash/commit/251fb5e69) Slightly optimize ApproximateBestSubset and its usage for PS txes (#3184)
- [`a55624b25`](https://github.com/dashpay/dash/commit/a55624b25) Fix 3182: Append scrollbar styles (#3186)
- [`1bbe1adb4`](https://github.com/dashpay/dash/commit/1bbe1adb4) Add a simple test for payoutAddress reuse in `protx update_registrar` (#3183)
- [`372389d23`](https://github.com/dashpay/dash/commit/372389d23) Disable styling for scrollbars on macos (#3182)
- [`e0781095f`](https://github.com/dashpay/dash/commit/e0781095f) A couple of fixes for additional indexes (#3181)
- [`d3ce0964b`](https://github.com/dashpay/dash/commit/d3ce0964b) Add Qt GUI refresh w/branding updates (#3000)
- [`9bc699ff2`](https://github.com/dashpay/dash/commit/9bc699ff2) Update activemn if protx info changed (#3176)
- [`bbd9b10d4`](https://github.com/dashpay/dash/commit/bbd9b10d4) Refactor nonLockedTxsByInputs (#3178)
- [`64a913d6f`](https://github.com/dashpay/dash/commit/64a913d6f) Allow empty strings in `protx update_registrar` as an option to re-use current values (#3177)
- [`3c21d2577`](https://github.com/dashpay/dash/commit/3c21d2577) Slightly adjust some README.md files (#3175)
- [`883fcbe8b`](https://github.com/dashpay/dash/commit/883fcbe8b) Always run extended tests in Gitlab CI (#3173)
- [`a7492c1d3`](https://github.com/dashpay/dash/commit/a7492c1d3) Handle coin type via CCoinControl (#3172)
- [`0d1a04905`](https://github.com/dashpay/dash/commit/0d1a04905) Don't show individual messages for each TX when too many come in at once (#3170)
- [`589c89250`](https://github.com/dashpay/dash/commit/589c89250) Fix 2 more bottlenecks causing GUI lockups (#3169)
- [`dfd6ee472`](https://github.com/dashpay/dash/commit/dfd6ee472) Actually update spent index on DisconnectBlock (#3167)
- [`3c818e95b`](https://github.com/dashpay/dash/commit/3c818e95b) Only track last seen time instead of first and last seen time (#3165)
- [`df3dbe85b`](https://github.com/dashpay/dash/commit/df3dbe85b) Wait for sporks to propagate in llmq-chainlocks.py before mining new blocks (#3168)
- [`8cbd63d9e`](https://github.com/dashpay/dash/commit/8cbd63d9e) Make HD wallet warning a bit more natural (#3164)
- [`001c4338b`](https://github.com/dashpay/dash/commit/001c4338b) Improved messaging for ip address errors (#3163)
- [`33d04ebf2`](https://github.com/dashpay/dash/commit/33d04ebf2) Disable move ctor/operator for CKeyHolder (#3162)
- [`da2f503a4`](https://github.com/dashpay/dash/commit/da2f503a4) Fix largest part of GUI lockups with large wallets (#3155)
- [`3c6b5f98e`](https://github.com/dashpay/dash/commit/3c6b5f98e) Use wallet UTXOs whenever possible to avoid looping through all wallet txes (#3156)
- [`4db91c605`](https://github.com/dashpay/dash/commit/4db91c605) Fix Gitlab cache issues (#3160)
- [`e9ed35482`](https://github.com/dashpay/dash/commit/e9ed35482) Partially revert 3061 (#3150)
- [`4b6af8f2c`](https://github.com/dashpay/dash/commit/4b6af8f2c) Few fixes related to SelectCoinsGroupedByAddresses (#3144)
- [`859d60f81`](https://github.com/dashpay/dash/commit/859d60f81) Don't use $CACHE_DIR in after_script (#3159)
- [`be127bc2e`](https://github.com/dashpay/dash/commit/be127bc2e) Replace vecAskFor with a priority queue (#3147)
- [`a13a9182d`](https://github.com/dashpay/dash/commit/a13a9182d) Add missing "notfound" and "getsporks" to messagemap (#3146)
- [`efd8d2c82`](https://github.com/dashpay/dash/commit/efd8d2c82) Avoid propagating InstantSend related old recovered sigs (#3145)
- [`24fee3051`](https://github.com/dashpay/dash/commit/24fee3051) Add support for Gitlab CI (#3149)
- [`1cbe280ad`](https://github.com/dashpay/dash/commit/1cbe280ad) Qt: Remove old themes (#3141)
- [`dcdf1f3a6`](https://github.com/dashpay/dash/commit/dcdf1f3a6) Some refactoring for spork related functionality in tests (#3137)
- [`411241471`](https://github.com/dashpay/dash/commit/411241471) Introduce getprivatesendinfo and deprecate getpoolinfo (#3140)
- [`152c10bc4`](https://github.com/dashpay/dash/commit/152c10bc4) Various fixes for mixing queues (#3138)
- [`e0c56246f`](https://github.com/dashpay/dash/commit/e0c56246f) Fixes and refactorings related to using mnsync in tests (#3136)
- [`f8e238c5b`](https://github.com/dashpay/dash/commit/f8e238c5b) [Trivial] RPC help updates (#3134)
- [`d49ee618f`](https://github.com/dashpay/dash/commit/d49ee618f) Add more logging to DashTestFramework (#3130)
- [`cd6c5b4b4`](https://github.com/dashpay/dash/commit/cd6c5b4b4) Multiple fixes for ChainLock tests (#3129)
- [`e06c116d2`](https://github.com/dashpay/dash/commit/e06c116d2) Actually pass extra_args to nodes in assumevalid.py (#3131)
- [`737ac967f`](https://github.com/dashpay/dash/commit/737ac967f) Refactor some Dash-specific `wait_for*` functions in tests (#3122)
- [`b4aefb513`](https://github.com/dashpay/dash/commit/b4aefb513) Also consider txindex for transactions in AlreadyHave() (#3126)
- [`d9e98e31e`](https://github.com/dashpay/dash/commit/d9e98e31e) Fix scripted diff check condition (#3128)
- [`bad3243b8`](https://github.com/dashpay/dash/commit/bad3243b8) Bump mocktime before generating new blocks and generate a few blocks at the end of `test_mempool_doublespend` in `p2p-instantsend.py` (#3125)
- [`82ebba18f`](https://github.com/dashpay/dash/commit/82ebba18f) Few fixes for `wait_for_instantlock` (#3123)
- [`a2fa9bb7e`](https://github.com/dashpay/dash/commit/a2fa9bb7e) Ignore recent rejects filter for locked txes (#3124)
- [`2ca2138fc`](https://github.com/dashpay/dash/commit/2ca2138fc) Whitelist nodes in llmq-dkgerrors.py (#3112)
- [`a8fa5cff9`](https://github.com/dashpay/dash/commit/a8fa5cff9) Make orphan TX map limiting dependent on total TX size instead of TX count (#3121)
- [`746b5f8cd`](https://github.com/dashpay/dash/commit/746b5f8cd) Remove commented out code (#3117)
- [`3ac583cce`](https://github.com/dashpay/dash/commit/3ac583cce) docs: Add packages for building in Alpine Linux (#3115)
- [`c5da93851`](https://github.com/dashpay/dash/commit/c5da93851) A couple of minor improvements in IS code (#3114)
- [`43b7c31d9`](https://github.com/dashpay/dash/commit/43b7c31d9) Wait for the actual best block chainlock in llmq-chainlocks.py (#3109)
- [`22ac6ba4e`](https://github.com/dashpay/dash/commit/22ac6ba4e) Make sure chainlocks and blocks are propagated in llmq-is-cl-conflicts.py before moving to next steps (#3108)
- [`9f1ee8c70`](https://github.com/dashpay/dash/commit/9f1ee8c70) scripted-diff: Refactor llmq type consensus param names (#3093)
- [`1c74b668b`](https://github.com/dashpay/dash/commit/1c74b668b) Introduce getbestchainlock rpc and fix llmq-is-cl-conflicts.py (#3094)
- [`ac0270871`](https://github.com/dashpay/dash/commit/ac0270871) Respect `logips` config option in few more log outputs (#3078)
- [`d26b6a84c`](https://github.com/dashpay/dash/commit/d26b6a84c) Fix a couple of issues with PS fee calculations (#3077)
- [`40399fd97`](https://github.com/dashpay/dash/commit/40399fd97) Circumvent BIP69 sorting in fundrawtransaction.py test (#3100)
- [`e2d651f60`](https://github.com/dashpay/dash/commit/e2d651f60) Add OpenSSL termios fix for musl libc (#3099)
- [`783653f6a`](https://github.com/dashpay/dash/commit/783653f6a) Ensure execinfo.h and linker flags set in autoconf (#3098)
- [`7320c3da2`](https://github.com/dashpay/dash/commit/7320c3da2) Refresh zmq 4.1.5 patches (#3092)
- [`822e617be`](https://github.com/dashpay/dash/commit/822e617be) Fix chia_bls include prefix (#3091)
- [`35f079cbf`](https://github.com/dashpay/dash/commit/35f079cbf) Remove unused code (#3097)
- [`1acde17e8`](https://github.com/dashpay/dash/commit/1acde17e8) Don't care about governance cache while the blockchain isn't synced yet (#3089)
- [`0d126c2ae`](https://github.com/dashpay/dash/commit/0d126c2ae) Use chainparams factory for devnet (#3087)
- [`ac90abe89`](https://github.com/dashpay/dash/commit/ac90abe89) When mixing, always try to join an exsisting queue, only fall back to starting a new queue (#3085)
- [`68d575dc0`](https://github.com/dashpay/dash/commit/68d575dc0) Masternodes should have no wallet enabled (#3084)
- [`6b5b70fab`](https://github.com/dashpay/dash/commit/6b5b70fab) Remove liquidity provider privatesend (#3082)
- [`0b2221ed6`](https://github.com/dashpay/dash/commit/0b2221ed6) Clarify default max peer connections (#3081)
- [`c22169d57`](https://github.com/dashpay/dash/commit/c22169d57) Reduce non-debug PS log output (#3076)
- [`41ae1c7e2`](https://github.com/dashpay/dash/commit/41ae1c7e2) Add LDFLAGS_WRAP_EXCEPTIONS to pigeon_fuzzy linking (#3075)
- [`77b88558e`](https://github.com/dashpay/dash/commit/77b88558e) Update/modernize macOS plist (#3074)
- [`f1ff14818`](https://github.com/dashpay/dash/commit/f1ff14818) Fix bip69 vs change position issue (#3063)
- [`9abc39383`](https://github.com/dashpay/dash/commit/9abc39383) Refactor few things here and there (#3066)
- [`3d5eabcfb`](https://github.com/dashpay/dash/commit/3d5eabcfb) Update/unify `debug` and `logging` rpc descriptions (#3071)
- [`0e94e97cc`](https://github.com/dashpay/dash/commit/0e94e97cc) Add missing tx `type` to `TxToUniv` (#3069)
- [`becca24fc`](https://github.com/dashpay/dash/commit/becca24fc) Few fixes in docs/comments (#3068)
- [`9d109d6a3`](https://github.com/dashpay/dash/commit/9d109d6a3) Add missing `instantlock`/`instantlock_internal` to `getblock`'s `verbosity=2` mode (#3067)
- [`0f088d03a`](https://github.com/dashpay/dash/commit/0f088d03a) Change regtest and devnet p2p/rpc ports (#3064)
- [`190542256`](https://github.com/dashpay/dash/commit/190542256) Rework govobject/trigger cleanup a bit (#3070)
- [`386de78bc`](https://github.com/dashpay/dash/commit/386de78bc) Fix SelectCoinsMinConf to allow instant respends (#3061)
- [`cbbeec689`](https://github.com/dashpay/dash/commit/cbbeec689) RPC Getrawtransaction fix (#3065)
- [`1e3496799`](https://github.com/dashpay/dash/commit/1e3496799) Added getmemoryinfo parameter string update (#3062)
- [`9d2d8cced`](https://github.com/dashpay/dash/commit/9d2d8cced) Add a few malleability tests for DIP2/3 transactions (#3060)
- [`4983f7abb`](https://github.com/dashpay/dash/commit/4983f7abb) RPC Fix typo in getmerkleblocks help (#3056)
- [`a78dcfdec`](https://github.com/dashpay/dash/commit/a78dcfdec) Add the public GPG key for Pasta for Gitian building (#3057)
- [`929c892c0`](https://github.com/dashpay/dash/commit/929c892c0) Remove p2p alert leftovers (#3050)
- [`dd7873857`](https://github.com/dashpay/dash/commit/dd7873857) Re-verify invalid IS sigs when the active quorum set rotated (#3052)
- [`13e023510`](https://github.com/dashpay/dash/commit/13e023510) Remove recovered sigs from the LLMQ db when corresponding IS locks get confirmed (#3048)
- [`4a7525da3`](https://github.com/dashpay/dash/commit/4a7525da3) Add "instantsendlocks" to getmempoolinfo RPC (#3047)
- [`fbb49f92d`](https://github.com/dashpay/dash/commit/fbb49f92d) Bail out properly on Evo DB consistency check failures in ConnectBlock/DisconnectBlock (#3044)
- [`8d89350b8`](https://github.com/dashpay/dash/commit/8d89350b8) Use less alarming fee warning note (#3038)
- [`02f6188e8`](https://github.com/dashpay/dash/commit/02f6188e8) Do not count 0-fee txes for fee estimation (#3037)
- [`f0c73f5ce`](https://github.com/dashpay/dash/commit/f0c73f5ce) Revert "Skip mempool.dat when wallet is starting in "zap" mode (#2782)"
- [`be3bc48c9`](https://github.com/dashpay/dash/commit/be3bc48c9) Fix broken link in PrivateSend info dialog (#3031)
- [`acab8c552`](https://github.com/dashpay/dash/commit/acab8c552) Add Dash Core Group codesign certificate (#3027)
- [`a1c4321e9`](https://github.com/dashpay/dash/commit/a1c4321e9) Fix osslsigncode compile issue in gitian-build (#3026)
- [`2f21e5551`](https://github.com/dashpay/dash/commit/2f21e5551) Remove legacy InstantSend code (#3020)
- [`7a440d626`](https://github.com/dashpay/dash/commit/7a440d626) Optimize on-disk deterministic masternode storage to reduce size of evodb (#3017)
- [`85fcf32c9`](https://github.com/dashpay/dash/commit/85fcf32c9) Remove support for InstantSend locked gobject collaterals (#3019)
- [`bdec34c94`](https://github.com/dashpay/dash/commit/bdec34c94) remove DS mixes once they have been included in a chainlocked block (#3015)
- [`ee9adb948`](https://github.com/dashpay/dash/commit/ee9adb948) Use std::unique_ptr for mnList in CSimplifiedMNList (#3014)
- [`b401a3baa`](https://github.com/dashpay/dash/commit/b401a3baa) Fix compilation on Ubuntu 16.04 (#3013)
- [`c6eededca`](https://github.com/dashpay/dash/commit/c6eededca) Add "isValidMember" and "memberIndex" to "quorum memberof" and allow to specify quorum scan count (#3009)
- [`b9aadc071`](https://github.com/dashpay/dash/commit/b9aadc071) Fix excessive memory use when flushing chainstate and EvoDB (#3008)
- [`780bffeb7`](https://github.com/dashpay/dash/commit/780bffeb7) Enable stacktrace support in gitian builds (#3006)
- [`5809c5c3d`](https://github.com/dashpay/dash/commit/5809c5c3d) Implement "quorum memberof" (#3004)
- [`63424fb26`](https://github.com/dashpay/dash/commit/63424fb26) Fix 2 common Travis failures which happen when Travis has network issues (#3003)
- [`09b017fc5`](https://github.com/dashpay/dash/commit/09b017fc5) Only load signingActiveQuorumCount + 1 quorums into cache (#3002)
- [`b75e1cebd`](https://github.com/dashpay/dash/commit/b75e1cebd) Decouple lite mode and client-side PrivateSend (#2893)
- [`b9a738528`](https://github.com/dashpay/dash/commit/b9a738528) Remove skipped denom from the list on tx commit (#2997)
- [`5bdb2c0ce`](https://github.com/dashpay/dash/commit/5bdb2c0ce) Revert "Show BIP9 progress in getblockchaininfo (#2435)"
- [`b62db7618`](https://github.com/dashpay/dash/commit/b62db7618) Revert " Add real timestamp to log output when mock time is enabled (#2604)"
- [`1f6e0435b`](https://github.com/dashpay/dash/commit/1f6e0435b) [Trivial] Fix a typo in a comment in mnauth.h (#2988)
- [`f84d5d46d`](https://github.com/dashpay/dash/commit/f84d5d46d) QT: Revert "Force TLS1.0+ for SSL connections" (#2985)
- [`2e13d1305`](https://github.com/dashpay/dash/commit/2e13d1305) Add some comments to make quorum merkle root calculation more clear+ (#2984)
- [`6677a614a`](https://github.com/dashpay/dash/commit/6677a614a) Run extended tests when Travis is started through cron (#2983)
- [`d63202bdc`](https://github.com/dashpay/dash/commit/d63202bdc) Should send "reject" when mixing queue is full (#2981)
- [`8d5781f40`](https://github.com/dashpay/dash/commit/8d5781f40) Stop reporting/processing the number of mixing participants in DSSTATUSUPDATE (#2980)
- [`7334aa553`](https://github.com/dashpay/dash/commit/7334aa553) adjust privatesend formatting and follow some best practices (#2979)
- [`f14179ca0`](https://github.com/dashpay/dash/commit/f14179ca0) [Tests] Remove unused variable and inline another variable in evo_deterministicmns_tests.cpp (#2978)
- [`2756cb795`](https://github.com/dashpay/dash/commit/2756cb795) remove spork 12 (#2754)
- [`633231092`](https://github.com/dashpay/dash/commit/633231092) Provide correct params to AcceptToMemoryPoolWithTime() in LoadMempool() (#2976)
- [`e03538778`](https://github.com/dashpay/dash/commit/e03538778) Back off for 1m when connecting to quorum masternodes (#2975)
- [`bfcfb70d8`](https://github.com/dashpay/dash/commit/bfcfb70d8) Ignore blocks that do not match the filter in getmerkleblocks rpc (#2973)
- [`4739daddc`](https://github.com/dashpay/dash/commit/4739daddc) Process/keep messages/connections from PoSe-banned MNs (#2967)
- [`864856688`](https://github.com/dashpay/dash/commit/864856688) Multiple speed optimizations for deterministic MN list handling (#2972)
- [`d931cb723`](https://github.com/dashpay/dash/commit/d931cb723) Update copyright date (2019) (#2970)
- [`a83b63186`](https://github.com/dashpay/dash/commit/a83b63186) Fix UI masternode list (#2966)
- [`85c9ea400`](https://github.com/dashpay/dash/commit/85c9ea400) Throw a bit more descriptive error message on UpgradeDB failure on pruned nodes (#2962)
- [`2c5e2bc6c`](https://github.com/dashpay/dash/commit/2c5e2bc6c) Inject custom specialization of std::hash for SporkId enum into std (#2960)
- [`809aae73a`](https://github.com/dashpay/dash/commit/809aae73a) RPC docs helper updates (#2949)
- [`09d66c776`](https://github.com/dashpay/dash/commit/09d66c776) Fix compiler warning (#2956)
- [`26bd0d278`](https://github.com/dashpay/dash/commit/26bd0d278) Fix bls and bls_dkg bench (#2955)
- [`d28d318aa`](https://github.com/dashpay/dash/commit/d28d318aa) Remove logic for handling objects and votes orphaned by not-yet-known MNs (#2954)
- [`e02c562aa`](https://github.com/dashpay/dash/commit/e02c562aa) [RPC] Remove check for deprecated `masternode start-many` command (#2950)
- [`fc73b4d6e`](https://github.com/dashpay/dash/commit/fc73b4d6e) Refactor sporks to get rid of repeated if/else blocks (#2946)
- [`a149ca747`](https://github.com/dashpay/dash/commit/a149ca747) Remove references to instantx and darksend in sendcoinsdialog.cpp (#2936)
- [`b74cd3e10`](https://github.com/dashpay/dash/commit/b74cd3e10) Only require valid collaterals for votes and triggers (#2947)
- [`66b336c93`](https://github.com/dashpay/dash/commit/66b336c93) Use Travis stages instead of custom timeouts (#2948)
- [`5780fa670`](https://github.com/dashpay/dash/commit/5780fa670) Remove duplicate code from src/Makefile.am (#2944)
- [`428f30450`](https://github.com/dashpay/dash/commit/428f30450) Implement `rawchainlocksig` and `rawtxlocksig` (#2930)
- [`c08e76101`](https://github.com/dashpay/dash/commit/c08e76101) Tighten rules for DSVIN/DSTX (#2897)
- [`f1fe24b67`](https://github.com/dashpay/dash/commit/f1fe24b67) Only gracefully timeout Travis when integration tests need to be run (#2933)
- [`7c05aa821`](https://github.com/dashpay/dash/commit/7c05aa821) Also gracefully timeout Travis builds when building source takes >30min (#2932)
- [`5652ea023`](https://github.com/dashpay/dash/commit/5652ea023) Show number of InstantSend locks in Debug Console (#2919)
- [`a3f030609`](https://github.com/dashpay/dash/commit/a3f030609) Implement getmerkleblocks rpc (#2894)
- [`32aa229c7`](https://github.com/dashpay/dash/commit/32aa229c7) Reorganize Dash Specific code into folders (#2753)
- [`acbf0a221`](https://github.com/dashpay/dash/commit/acbf0a221) Bump version to 0.14.1 (#2928)

External wallet files
---------------------

The `-wallet=<path>` option now accepts full paths instead of requiring wallets
to be located in the -walletdir directory.

Newly created wallet format
---------------------------

If `-wallet=<path>` is specified with a path that does not exist, it will now
create a wallet directory at the specified location (containing a wallet.dat
data file, a db.log file, and database/log.?????????? files) instead of just
creating a data file at the path and storing log files in the parent
directory. This should make backing up wallets more straightforward than
before because the specified wallet path can just be directly archived without
having to look in the parent directory for transaction log files.

For backwards compatibility, wallet paths that are names of existing data files
in the `-walletdir` directory will continue to be accepted and interpreted the
same as before.

Low-level RPC changes
---------------------

- When Dash Core is not started with any `-wallet=<path>` options, the name of
  the default wallet returned by `getwalletinfo` and `listwallets` RPCs is
  now the empty string `""` instead of `"wallet.dat"`. If Dash Core is started
  with any `-wallet=<path>` options, there is no change in behavior, and the
  name of any wallet is just its `<path>` string.

Credits
=======

Thanks to everyone who directly contributed to this release:

- 10xcryptodev
- Akshay CM (akshaynexus)
- Alexander Block (codablock)
- Cofresi
- CryptoTeller
- dustinface (xdustinface)
- konez2k
- Oleg Girko (OlegGirko)
- PastaPastaPasta
- Piter Bushnell (Bushstar)
- sc-9310
- thephez
- UdjinM6

As well as everyone that submitted issues and reviewed pull requests.

Older releases
==============

Dash was previously known as Darkcoin.

Darkcoin tree 0.8.x was a fork of Litecoin tree 0.8, original name was XCoin
which was first released on Jan/18/2014.

Darkcoin tree 0.9.x was the open source implementation of masternodes based on
the 0.8.x tree and was first released on Mar/13/2014.

Darkcoin tree 0.10.x used to be the closed source implementation of Darksend
which was released open source on Sep/25/2014.

Dash Core tree 0.11.x was a fork of Bitcoin Core tree 0.9,
Darkcoin was rebranded to Dash.

Dash Core tree 0.12.0.x was a fork of Bitcoin Core tree 0.10.

Dash Core tree 0.12.1.x was a fork of Bitcoin Core tree 0.12.

These release are considered obsolete. Old release notes can be found here:

- [v0.15.0.0](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.15.0.0.md) released Febrary/18/2020
- [v0.14.0.5](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.14.0.5.md) released December/08/2019
- [v0.14.0.4](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.14.0.4.md) released November/22/2019
- [v0.14.0.3](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.14.0.3.md) released August/15/2019
- [v0.14.0.2](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.14.0.2.md) released July/4/2019
- [v0.14.0.1](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.14.0.1.md) released May/31/2019
- [v0.14.0](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.14.0.md) released May/22/2019
- [v0.13.3](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.13.3.md) released Apr/04/2019
- [v0.13.2](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.13.2.md) released Mar/15/2019
- [v0.13.1](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.13.1.md) released Feb/9/2019
- [v0.13.0](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.13.0.md) released Jan/14/2019
- [v0.12.3.4](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.12.3.4.md) released Dec/14/2018
- [v0.12.3.3](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.12.3.3.md) released Sep/19/2018
- [v0.12.3.2](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.12.3.2.md) released Jul/09/2018
- [v0.12.3.1](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.12.3.1.md) released Jul/03/2018
- [v0.12.2.3](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.12.2.3.md) released Jan/12/2018
- [v0.12.2.2](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.12.2.2.md) released Dec/17/2017
- [v0.12.2](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.12.2.md) released Nov/08/2017
- [v0.12.1](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.12.1.md) released Feb/06/2017
- [v0.12.0](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.12.0.md) released Aug/15/2015
- [v0.11.2](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.11.2.md) released Mar/04/2015
- [v0.11.1](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.11.1.md) released Feb/10/2015
- [v0.11.0](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.11.0.md) released Jan/15/2015
- [v0.10.x](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.10.0.md) released Sep/25/2014
- [v0.9.x](https://github.com/dashpay/dash/blob/master/doc/release-notes/dash/release-notes-0.9.0.md) released Mar/13/2014
