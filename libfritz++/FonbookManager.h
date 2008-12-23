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


#ifndef FONBUCHMANAGER_H_
#define FONBUCHMANAGER_H_

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
	 *
	 */
	static Fonbook *GetFonbook();
	/**
	 *
	 */
	static FonbookManager *GetFonbookManager();
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
	 * @return the number of entries or cFonbuch::npos, if requesting specific telephonebook entries is not possible for this telephonebook
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

#endif /*FONBUCHMANAGER_H_*/
