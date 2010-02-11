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
	// track the currently active (=shown) fonbook
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
		*dsyslog << __FILE__ << ": deleting fonbook with ID: " << fonbooks[i]->GetTechId() << std::endl;
		delete(fonbooks[i]);
	}
}

void FonbookManager::CreateFonbookManager( std::vector <std::string> vFonbookID, std::string activeFonbook){
	if (gConfig) {
		// if there is already a FonbookManager, delete it, so it can adapt to configuration changes
		DeleteFonbookManager();
		// save new list of fonbook ids
		gConfig->setFonbookIDs(vFonbookID);
		// check if activeFonbook is valid
		if (activeFonbook.size() > 0) {
			bool activeFonbookValid = false;
			for (unsigned int pos = 0; pos < vFonbookID.size(); pos++)
				if (vFonbookID[pos].compare(activeFonbook) == 0) {
					activeFonbookValid = true;
					break;
				}
			if (activeFonbookValid)
				gConfig->setActiveFonbook(activeFonbook);
			else
				*esyslog << __FILE__ << ": Invalid call parameter. ActiveFonbook '" << activeFonbook << "'is not enabled or unknown" << std::endl;
		}
		// create fonbookmanger (was deleted above) so that it can initialize all fonbooks
		me = new FonbookManager();
	} else {
		*esyslog << __FILE__ << ": Wrong call sequence. Configuration does not exist when trying to create FonbookManager."  << std::endl;
	}
}

Fonbook* FonbookManager::GetFonbook() {
	if (!me)
		me = new FonbookManager();
	return (Fonbook*) me;
}

FonbookManager* FonbookManager::GetFonbookManager() {
	if (!me)
		me = new FonbookManager();
	return me;
}

void FonbookManager::DeleteFonbookManager() {
	if (me) {
		*dsyslog << __FILE__ << ": deleting Fonbook Manager" << std::endl;
		delete me;
		me = NULL;
	}
}

void FonbookManager::NextFonbook() {
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
		NextFonbook(); //todo: what if no fonebook is configured at all?
	}
	return fonbooks[gConfig->getFonbookIDs()[activeFonbookPos]];
}

FonbookEntry *FonbookManager::RetrieveFonbookEntry(size_t id) {
	return GetActiveFonbook() ? GetActiveFonbook()->RetrieveFonbookEntry(id) : NULL;
}

bool FonbookManager::AddFonbookEntry(FonbookEntry fe) {
	return GetActiveFonbook() ? GetActiveFonbook()->AddFonbookEntry(fe) : false;
}

bool FonbookManager::isDisplayable() {
	return GetActiveFonbook() ? GetActiveFonbook()->isDisplayable() : false;
}

bool FonbookManager::isInitialized() {
	return GetActiveFonbook() ? GetActiveFonbook()->isInitialized() : false;
}

bool FonbookManager::isWriteable() {
	return GetActiveFonbook() ? GetActiveFonbook()->isWriteable() : false;
}

void FonbookManager::setInitialized(bool isInitialized) {
	if (GetActiveFonbook())
		GetActiveFonbook()->setInitialized(isInitialized);
}

void FonbookManager::Sort(FonbookEntry::eElements element, bool ascending){
	if (GetActiveFonbook())
		GetActiveFonbook()->Sort(element, ascending);
}

size_t FonbookManager::GetFonbookSize() {
	return GetActiveFonbook() ? GetActiveFonbook()->GetFonbookSize() : 0;
}

std::string FonbookManager::GetTitle() {
	return GetActiveFonbook() ? GetActiveFonbook()->GetTitle() : "";
}

std::string FonbookManager::GetTechId() {
	return GetActiveFonbook() ? GetActiveFonbook()->GetTechId() : "";
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
