/*
 * libfritz++
 *
 * Copyright (C) 2007-2010 Joachim Wilke <libfritz@joachim-wilke.de>
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

namespace fritz {

/**
 * General telephonebook entry.
 * This defines the class, to be used by every phone book implementation.
 */

class FonbookEntry {
public:
	enum eType {
		TYPE_NONE,
		TYPE_HOME,
		TYPE_MOBILE,
		TYPE_WORK,
		TYPES_COUNT
	};

	enum eElements {
		ELEM_NAME   = 0,
		ELEM_TYPE   = 1,
		ELEM_NUMBER = 2,
		ELEM_IMPORTANT,
		ELEM_QUICKDIAL,
		ELEM_VANITY,
		ELEM_PRIORITY,
		ELEMS_COUNT
	};
	struct sNumber {
		std::string number;
		std::string quickdial;
		std::string vanity;
		int priority;
	};
private:
	std::string name;
	bool important;
	sNumber numbers[TYPES_COUNT];
public:
	/*
	 * Constructs a new FonbookEntry object
	 * @param name Full name of contact
	 * @param important Whether contact is flagged as important
	 */
	FonbookEntry(std::string name, bool important = false);
	/**
	 * Adds new number to this contact
	 * @param number The number to be added
	 * @param type The number type
	 * @param quickdial The quickdial extension
	 * @param vanity The vanity extension
	 * @param prority '1' marks the default number of this contact, otherwise 0
	 */
	void addNumber(std::string number, eType type = TYPE_NONE, std::string quickdial = "", std::string vanity = "", int priority = 0);
	std::string getName() const { return name; }
	void setName(std::string name) { this->name = name; }
	std::string getNumber(eType type) const { return numbers[type].number; }
	void setNumber(std::string number, eType type) { numbers[type].number = number; }
	bool isImportant() { return important; }
	void setImportant(bool important) { this->important = important; }
	eType getDefaultType();
	void setDefaultType(eType type);
	std::string getQuickdialFormatted(eType type = TYPES_COUNT);
	std::string getQuickdial(eType type = TYPES_COUNT);
	void setQuickdial(std::string quickdial, eType type = TYPES_COUNT);
	std::string getVanity(eType type = TYPES_COUNT);
	std::string getVanityFormatted(eType type = TYPES_COUNT);
	void setVanity(std::string vanity, eType type = TYPES_COUNT);
	int getPriority(eType type) { return numbers[type].priority; }
	void setPrioriy(int priority, eType type) { numbers[type].priority = priority; }
	bool operator<(const FonbookEntry & fe) const;
	/*
	 * Get number of typed numbers (TYPE_NONE is ignored)
	 * @return count of different numbers available
	 */
	size_t getSize();
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
	 * Method to persist contents of the phone book (if writeable)
	 */
	virtual void Save() {}
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
	struct sResolveResult {
		std::string name;
		FonbookEntry::eType type;
	};
	virtual ~Fonbook() { Save(); }
	/**
	 * Take action to fill phonebook with content.
	 * Initialize() may be called more than once per session.
	 * @return if initialization was successful
	 */
	virtual bool Initialize(void) { return true; }
	/**
	 * Resolves the number given to the corresponding name.
	 * @param number to resolve
	 * @return resolved name and type or the number, if unsuccessful
	 */
	virtual sResolveResult ResolveToName(std::string number);
	/**
	 * Returns a specific telephonebook entry.
	 * @param id unique identifier of the requested entry
	 * @return the entry with key id or NULL, if unsuccessful
	 */
	virtual FonbookEntry *RetrieveFonbookEntry(size_t id);
	/**
	 * Adds a new entry to the phonebook.
	 * @param fe a new phonebook entry
	 * @return true, if add was successful
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
	virtual std::string GetTechId() { return techId; }
	/**
	 * Sorts the phonebook's entries by the given element and in given order.
	 * @param the element used for sorting
	 * @param true if sort order is ascending, false otherwise
	 */
	virtual void Sort(FonbookEntry::eElements element = FonbookEntry::ELEM_NAME, bool ascending = true);
};

}

#endif /*FONBOOK_H_*/
