// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "uritests.h"

#include "guiutil.h"
#include "walletmodel.h"

#include <QUrl>

void URITests::uriTests()
{
    SendCoinsRecipient rv;
    QUrl uri;
    uri.setUrl(QString("pigeon:PSkeoPYpXT43crZSLwMV9jEnq9aKbFUyLt?req-dontexist="));
    QVERIFY(!GUIUtil::parseBitcoinURI(uri, &rv));

    uri.setUrl(QString("pigeon:PSkeoPYpXT43crZSLwMV9jEnq9aKbFUyLt?dontexist="));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    QVERIFY(rv.address == QString("PSkeoPYpXT43crZSLwMV9jEnq9aKbFUyLt"));
    QVERIFY(rv.label == QString());
    QVERIFY(rv.amount == 0);

    uri.setUrl(QString("pigeon:PSkeoPYpXT43crZSLwMV9jEnq9aKbFUyLt?label=Some Example Address"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    QVERIFY(rv.address == QString("PSkeoPYpXT43crZSLwMV9jEnq9aKbFUyLt"));
    QVERIFY(rv.label == QString("Some Example Address"));
    QVERIFY(rv.amount == 0);

    uri.setUrl(QString("pigeon:PSkeoPYpXT43crZSLwMV9jEnq9aKbFUyLt?amount=0.001"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    QVERIFY(rv.address == QString("PSkeoPYpXT43crZSLwMV9jEnq9aKbFUyLt"));
    QVERIFY(rv.label == QString());
    QVERIFY(rv.amount == 100000);

    uri.setUrl(QString("pigeon:PSkeoPYpXT43crZSLwMV9jEnq9aKbFUyLt?amount=1.001"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    QVERIFY(rv.address == QString("PSkeoPYpXT43crZSLwMV9jEnq9aKbFUyLt"));
    QVERIFY(rv.label == QString());
    QVERIFY(rv.amount == 100100000);

    uri.setUrl(QString("pigeon:PSkeoPYpXT43crZSLwMV9jEnq9aKbFUyLt?amount=100&label=Some Example"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    QVERIFY(rv.address == QString("PSkeoPYpXT43crZSLwMV9jEnq9aKbFUyLt"));
    QVERIFY(rv.amount == 10000000000LL);
    QVERIFY(rv.label == QString("Some Example"));

    uri.setUrl(QString("pigeon:PSkeoPYpXT43crZSLwMV9jEnq9aKbFUyLt?message=Some Example Address"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    QVERIFY(rv.address == QString("PSkeoPYpXT43crZSLwMV9jEnq9aKbFUyLt"));
    QVERIFY(rv.label == QString());

    QVERIFY(GUIUtil::parseBitcoinURI("pigeon://PSkeoPYpXT43crZSLwMV9jEnq9aKbFUyLt?message=Some Example Address", &rv));
    QVERIFY(rv.address == QString("PSkeoPYpXT43crZSLwMV9jEnq9aKbFUyLt"));
    QVERIFY(rv.label == QString());

    uri.setUrl(QString("pigeon:PSkeoPYpXT43crZSLwMV9jEnq9aKbFUyLt?req-message=Some Example Address"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));

    uri.setUrl(QString("pigeon:PSkeoPYpXT43crZSLwMV9jEnq9aKbFUyLt?amount=1,000&label=Some Example"));
    QVERIFY(!GUIUtil::parseBitcoinURI(uri, &rv));

    uri.setUrl(QString("pigeon:PSkeoPYpXT43crZSLwMV9jEnq9aKbFUyLt?amount=1,000.0&label=Some Example"));
    QVERIFY(!GUIUtil::parseBitcoinURI(uri, &rv));

    uri.setUrl(QString("pigeon:PSkeoPYpXT43crZSLwMV9jEnq9aKbFUyLt?amount=100&label=Some Example&message=Some Example Message"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    QVERIFY(rv.address == QString("PSkeoPYpXT43crZSLwMV9jEnq9aKbFUyLt"));
    QVERIFY(rv.amount == 10000000000LL);
    QVERIFY(rv.label == QString("Some Example"));
    QVERIFY(rv.message == QString("Some Example Message"));

    // Verify that IS=xxx does not lead to an error (we ignore the field)
    uri.setUrl(QString("pigeon:PSkeoPYpXT43crZSLwMV9jEnq9aKbFUyLt?IS=1"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));

    uri.setUrl(QString("pigeon:PSkeoPYpXT43crZSLwMV9jEnq9aKbFUyLt?req-IS=1"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));

    uri.setUrl(QString("pigeon:PSkeoPYpXT43crZSLwMV9jEnq9aKbFUyLt"));
    QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
}
