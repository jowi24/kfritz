/*
 * libfritz++
 *
 * Copyright (C) 2007-2008 Joachim Wilke <vdr@joachim-wilke.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */


#include <string>
#include "FonbookManager.h"
#include "FritzFonbook.h"
#include "OertlichesFonbook.h"
#include "Nummerzoeker.h"
#include "LocalFonbook.h"
#include "Config.h"

namespace fritz{

FonbookManager* FonbookManager::me = NULL;

FonbookManager::FonbookManager()
{
	// create all fonbooks
	fonbooks.push_back(new FritzFonbook());
	fonbooks.push_back(new OertlichesFonbook());
	fonbooks.push_back(new NummerzoekerFonbook());
	fonbooks.push_back(new LocalFonbook());
	// initialize the fonbooks that are used
	for (int i=gConfig->getFonbookIDs().size()-1; i>=0; i--) {
		Fonbook *fb = fonbooks[gConfig->getFonbookIDs()[i]];
		if (fb)
			fb->Initialize();
		else
			gConfig->getFonbookIDs().erase(gConfig->getFonbookIDs().begin()+i);
	}
	// track the currently active (=shown) fonbuch
	activeFonbookPos = std::string::npos;
    // set activeFonbookPos to the last displayed fonbook (if this is still valid and displayable)
	size_t pos = 0;
	while (pos < gConfig->getFonbookIDs().size() &&
			gConfig->getFonbookIDs()[pos] != gConfig->getActiveFonbook() )
		pos++;
	if (pos < gConfig->getFonbookIDs().size()) {
		if (fonbooks[gConfig->getFonbookIDs()[pos]]->isDisplayable())
			activeFonbookPos = pos;
	}
}

FonbookManager::~FonbookManager()
{
	for (size_t i= 0; i < fonbooks.size(); i++) {
		delete(fonbooks[i]);
	}
}

Fonbook* FonbookManager::GetFonbuch() {
	if (!me)
		me = new FonbookManager();
	return (Fonbook*) me;
}

FonbookManager* FonbookManager::GetFonbuchManager() {
	if (!me)
		me = new FonbookManager();
	return me;
}

void FonbookManager::NextFonbuch() {
	size_t pos = activeFonbookPos + 1;
    // no phonebooks -> no switching
	if ( gConfig->getFonbookIDs().size() == 0)
		return;
	while (pos < gConfig->getFonbookIDs().size() &&
			fonbooks[gConfig->getFonbookIDs()[pos]]->isDisplayable() == false)
		pos++;
	// if no displayable fonbook found -> start from beginning
	if (pos == gConfig->getFonbookIDs().size()) {
		pos = 0;
		while (pos < gConfig->getFonbookIDs().size() &&
				fonbooks[gConfig->getFonbookIDs()[pos]]->isDisplayable() == false)
			pos++;
		// if this fails, too, just return 0
		if (pos == gConfig->getFonbookIDs().size()) {
			pos = 0;
		}
	}
	activeFonbookPos = pos;
	// save the tech-id of the active fonbook in setup
	gConfig->setActiveFonbook( gConfig->getFonbookIDs()[pos] );
}

FonbookEntry &FonbookManager::ResolveToName(FonbookEntry &fe) {
	for (size_t i=0; i<gConfig->getFonbookIDs().size(); i++) {
		fe = fonbooks[gConfig->getFonbookIDs()[i]]->ResolveToName(fe);
		*dsyslog << __FILE__ << " ResolveToName: " << gConfig->getFonbookIDs()[i] << " " << fe.getName() << std::endl;
		if (fe.getName().compare(fe.getNumber()) != 0)
			return fe;
	}
	return fe;
}

Fonbook *FonbookManager::GetActiveFonbook() {
	if (activeFonbookPos == std::string::npos) {
		NextFonbuch();
	}
	return fonbooks[gConfig->getFonbookIDs()[activeFonbookPos]];
}

FonbookEntry *FonbookManager::RetrieveFonbuchEntry(size_t id) {
	return GetActiveFonbook() ? GetActiveFonbook()->RetrieveFonbuchEntry(id) : NULL;
}

bool FonbookManager::isDisplayable() {
	return GetActiveFonbook() ? GetActiveFonbook()->isDisplayable() : false;
}

bool FonbookManager::isInitialized() {
	return GetActiveFonbook() ? GetActiveFonbook()->isInitialized() : false;
}

void FonbookManager::setInitialized(bool isInitialized) {
	if (GetActiveFonbook())
		GetActiveFonbook()->setInitialized(isInitialized);
}

size_t FonbookManager::GetFonbuchSize() {
	return GetActiveFonbook() ? GetActiveFonbook()->GetFonbuchSize() : 0;
}

std::string FonbookManager::GetTitle() {
	return GetActiveFonbook() ? GetActiveFonbook()->GetTitle() : "";
}

void FonbookManager::Reload() {
	for (size_t i=0; i<gConfig->getFonbookIDs().size(); i++) {
		fonbooks[gConfig->getFonbookIDs()[i]]->Reload();
	}
}

Fonbooks *FonbookManager::GetFonbooks() {
	return &fonbooks;
}

}
