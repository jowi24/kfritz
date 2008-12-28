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


#ifndef FONBOOKMANAGER_H_
#define FONBOOKMANAGER_H_

#include <string>
#include "Fonbook.h"
#include "Fonbooks.h"

namespace fritz{

class FonbookManager : public Fonbook
{
private:
	static FonbookManager* me;
	Fonbooks fonbooks;
	FonbookManager();
	Fonbook *GetActiveFonbook();
	size_t activeFonbookPos;
public:
	virtual ~FonbookManager();
	/**
	 * Creates the central FonbookManager and activates certain fonbooks.
	 * This method instantiates the fonbookmanager. Following calls to
	 * getFonbookManager() return a reference to this object.
	 * CreateFonbookManager should be called before any call to getFonbookManager() to allow
	 * the configured fonbooks to initialize and fetch data which may be done in separate threads.
	 * If some of the fonbooks provided by libfritz++ shall be used, they need to be
	 * activated by this method. These fonbooks are used for reverse lookup on call events.
	 * The order of the fonbooks determines the priority regarding these lookups.
	 * Regarding queries to the fonbooks, a pointer is maintained which points to the currently
	 * "active" fonbook. This pointer can be moved, using FonbookManager::NextFonbook().
	 * @param the list of enabled fonbooks
	 * @param the currently "active" fonbook
	 */
	static void CreateFonbookManager( std::vector <std::string> vFonbookID, std::string activeFonbook);
	/**
	 * Returns the instance object of the FonbookManager casted to Fonbook.
	 */
	static Fonbook *GetFonbook();
	/**
	 * Returns the instance object of the FonbookManager
	 */
	static FonbookManager *GetFonbookManager();
	/*
	 * Deletes the FonbookManager instance.
	 */
	static void DeleteFonbookManager();
	/**
	 * Switch to next displayable phonebook.
	 * @return void
	 */
	void NextFonbook();
	/**
	 * Resolves the given number to the corresponding name.
	 * @param number the number to resolve
	 * @return the resolved name or the number, if unsuccesful
	 */
	FonbookEntry &ResolveToName(FonbookEntry &fe);
	/**
	 * Returns a specific telephonebook entry.
	 * @param id unique identifier of the requested entry
	 * @return the entry with key id or NULL, if unsuccesful
	 */
	FonbookEntry *RetrieveFonbookEntry(size_t id);
	/**
	 * Returns if it is possible to display the entries of this phonebook.
	 * @return true, if this phonebook has displayable entries. "Reverse lookup only" phonebooks must return false here.
	 */
	virtual bool isDisplayable();
	/**
	 * Returns if this phonebook is ready to use.
	 * @return true, if this phonebook is ready to use
	 */
	virtual bool isInitialized();
	/**
	 * Sets the initialized-status.
	 * @param isInititalized the value initialized is set to
	 */
	virtual void setInitialized(bool isInitialized);
	/**
	 *  Returns the number of entries in the telephonebook.
	 * @return the number of entries or cFonbook::npos, if requesting specific telephonebook entries is not possible for this telephonebook
	 */
	size_t GetFonbookSize();
	/**
	 *  Reloads the telephonebook's content
	 */
	void Reload();
	/**
	 *  Returns a string that should be displayed as title in the menu when the telephonebook is displayed.
	 */
	std::string GetTitle();
	/**
	 *
	 */
	Fonbooks *GetFonbooks();
};

}

#endif /*FONBOOKMANAGER_H_*/
