# Seeds

Utility to generate the seeds.txt list that is compiled into the client
(see [src/chainparamsseeds.h](/src/chainparamsseeds.h) and other utilities in [contrib/seeds](/contrib/seeds)).

The seeds compiled into the release are created from the current protx list, like this:

The seeds compiled into the release are created from the current masternode list, like this:

    pigeon-cli masternodelist full > mnlist.json
    python3 makeseeds.py < mnlist.json > nodes_main.txt
    python3 generate-seeds.py . > ../../src/chainparamsseeds.h

Make sure to use a recent block height in the "protx list" call. After updating, create a PR and
specify which block height you used so that reviewers can re-run the same commands and verify
that the list is as expected.

## Dependencies

Ubuntu:

    sudo apt-get install python3-dnspython
