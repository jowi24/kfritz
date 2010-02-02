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


#ifndef FONBOOK_H_
#define FONBOOK_H_

#include <string>
#include <vector>

namespace fritz{

/**
 * General telephonebook entry.
 * This definies the class, to be used by every telephonebook implementation.
 */

class FonbookEntry {
public:
	enum eType {
		TYPE_NONE,
		TYPE_HOME,
		TYPE_MOBILE,
		TYPE_WORK
	};
	enum eElements {
		ELEM_NAME   = 0,
		ELEM_TYPE   = 1,
		ELEM_NUMBER = 2,
	};
private:
	std::string name;
	std::string number;
	eType type;
public:
	FonbookEntry(std::string name, std::string number, eType type = TYPE_NONE);
	std::string getName() const { return name; }
	void setName(std::string name) { this->name = name; }
	std::string getNumber() const { return number; }
	eType getType() const { return type; }
	void setType(eType type) { this->type = type; }
	bool operator<(const FonbookEntry & fe) const;
};

/**
 * General telephonebook base class.
 * All specific telephonebooks have to inherit from this class.
 */

class Fonbook
{
private:
	/**
	 * True, if this phonebook is ready to use.
	 */
	bool initialized;
protected:
	/**
	 * The constructor may only be used by cFonbookManager.
	 * Subclasses must make their constructor private, too.
	 */
	Fonbook();
	/**
	 * The descriptive title of this phonebook.
	 */
	std::string title;
	/**
	 * The technical id of this phonebook (should be a short letter code).
	 */
	std::string techId;
	/**
	 * True, if this phonebook has displayable entries.
	 */
	bool displayable;
	/**
	 * True, if this phonebook is writeable
	 */
	bool writeable;
    /**
     * Data structure for storing the phonebook.
     */
	std::vector<FonbookEntry> fonbookList;
public:
	virtual ~Fonbook() {}
	/**
	 * Take action to fill phonebook with content.
	 * Initialize() may be called more than once per session.
	 * @return if initialization was successful
	 */
	virtual bool Initialize(void) { return true; }
	/**
	 * Resolves the number given in the FonbookEntry to the corresponding name.
	 * @param the fe containing the number the number to resolve
	 * @return the same fe containing resolved name or the number, if unsuccesful
	 */
	virtual FonbookEntry &ResolveToName(FonbookEntry &fe);
	/**
	 * Returns a specific telephonebook entry.
	 * @param id unique identifier of the requested entry
	 * @return the entry with key id or NULL, if unsuccesful
	 */
	virtual FonbookEntry *RetrieveFonbookEntry(size_t id);
	/**
	 * Adds a new entry to the phonebook.
	 * @param fe a new phonebook entry
	 * @return true, if add was sucessful
	 */
	virtual bool AddFonbookEntry(FonbookEntry fe __attribute__((unused))) { return false; }
	/**
	 * Returns if it is possible to display the entries of this phonebook.
	 * @return true, if this phonebook has displayable entries. "Reverse lookup only" phonebooks must return false here.
	 */
	virtual bool isDisplayable() { return displayable; }
	/**
	 * Returns if this phonebook is ready to use.
	 * @return true, if this phonebook is ready to use
	 */
	virtual bool isInitialized() { return initialized; }
	/**
	 * Returns if this phonebook is writeable, e.g. entries can be added or modified.
	 * @return true, if this phonebook is writeable
	 */
	virtual bool isWriteable() { return writeable; }
	/**
	 * Sets the initialized-status.
	 * @param isInititalized the value initialized is set to
	 */
	virtual void setInitialized(bool isInitialized) { initialized = isInitialized; }
	/**
	 *  Returns the number of entries in the telephonebook.
	 * @return the number of entries
	 */
	virtual size_t GetFonbookSize();
	/**
	 *  Reloads the telephonebook's content
	 */
	virtual void Reload() { }
	/**
	 *  Returns a string that should be displayed as title in the menu when the telephonebook is displayed.
	 * @return the long title of this phonebook
	 */
	virtual std::string GetTitle() { return title; }
	/**
	 * Returns the technical id of this phonebook. This id has to be unique among all phonebooks and is used when storing
	 * the plugin's setup.
	 * @return the technical id
	 */
	virtual std::string &GetTechId() { return techId; }
	/**
	 * Sorts the phonebook's entries by the given element and in given order.
	 * @param the element used for sorting
	 * @param true if sort order is ascending, false otherwise
	 */
	virtual void Sort(FonbookEntry::eElements element = FonbookEntry::ELEM_NAME, bool ascending = true);
};

}

#endif /*FONBOOK_H_*/
