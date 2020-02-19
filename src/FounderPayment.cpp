/*
 * Copyright (c) 2018 The Pigeon Core developers
 * Distributed under the MIT software license, see the accompanying
 * file COPYING or http://www.opensource.org/licenses/mit-license.php.
 * 
 * FounderPayment.cpp
 *
 *  Created on: Jun 24, 2018
 *      Author: Tri Nguyen
 */

#include "FounderPayment.h"
//#include "standard.h";
#include "util.h"
#include "chainparams.h"
#include <boost/foreach.hpp>
#include "base58.h"

CAmount FounderPayment::getFounderPaymentAmount(int blockHeight, CAmount blockReward) {
	 if (blockHeight <= startBlock){
		 return 0;
	 }
	 for(int i = 0; i < rewardStructures.size(); i++) {
		 FounderRewardStrcuture rewardStructure = rewardStructures[i];
		 if(blockHeight <= rewardStructure.blockHeight) {
			 return blockReward * rewardStructure.rewardPercentage / 100;
		 }
	 }
	 return 0;
}
CScript FounderPayment::GetFounderPayeeScript(int nHeight){
	string payeeaddr = nHeight >= address2StartBlock ? founderAddress2 : founderAddress;
	CBitcoinAddress devfeeaddr(payeeaddr);
	return GetScriptForDestination(devfeeaddr.Get());
}
void FounderPayment::FillFounderPayment(CMutableTransaction& txNew, int nBlockHeight, CAmount blockReward, CTxOut& txoutFounderRet) {
    // make sure it's not filled yet
	txoutFounderRet = CTxOut();
    // GET FOUNDER PAYMENT VARIABLES SETUP
    CAmount founderPayment = getFounderPaymentAmount(nBlockHeight, blockReward);
    // split reward between miner ...
    txNew.vout[0].nValue -= founderPayment;
    txoutFounderRet = CTxOut(founderPayment, GetFounderPayeeScript(nBlockHeight));
    txNew.vout.push_back(txoutFounderRet);
    LogPrintf("FounderPayment::FillFounderPayment -- Founder payment %lld to %s\n", founderPayment,
    		nBlockHeight >= address2StartBlock ? founderAddress2.c_str() : founderAddress.c_str());
}

bool FounderPayment::IsBlockPayeeValid(const CTransaction& txNew, const int height, const CAmount blockReward) {
	CScript payee;
	// fill payee with the founder address
	string payeeaddr = height >= address2StartBlock ? founderAddress2 : founderAddress;
	CBitcoinAddress devfeeaddr(payeeaddr);
	payee = GetScriptForDestination(devfeeaddr.Get());
	const CAmount founderReward = getFounderPaymentAmount(height, blockReward);
	BOOST_FOREACH(const CTxOut& out, txNew.vout) {
		if(out.scriptPubKey == GetFounderPayeeScript(height) && out.nValue >= founderReward) {
			return true;
		}
	}

	return false;
}



