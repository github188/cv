/*
 * WengoPhone, a voice over Internet phone
 * Copyright (C) 2004-2006  Wengo
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef IMCONTACTLISTHANDLER_H
#define IMCONTACTLISTHANDLER_H

#include <imwrapper/IMContact.h>

#include <util/Event.h>
#include <util/Trackable.h>

#include <map>
#include <set>
#include <string>

class IMAccount;
class IMAccountManager;
class IMContactList;
class UserProfile;

class IMContactListHandler : public Trackable {
public:

	/**
	 * Emitted when an IMContact has been added remotely.
	 *
	 * @param sender this class
	 * @param groupName the group where the IMContact has been added
	 * @param newIMContact the new IMContact
	 * @see IMContactList::newContactAddedEvent
	 */
	Event< void (IMContactListHandler & sender, const std::string & groupName,
		IMContact & newIMContact) > newIMContactAddedEvent;

	/**
	 * Emitted when an IMContact has been removed remotely.
	 *
	 * @param sender this class
	 * @param groupName the group where the IMContact is
	 * @param imContact the removed IMContact
	 * @see IMContactList::imContactRemovedEvent
	 */
	Event< void (IMContactListHandler & sender, const std::string & groupName,
		IMContact & imContact) > imContactRemovedEvent;

	/**
	 * @see IMContactList::contactMovedEvent
	 */
	Event< void (IMContactListHandler & sender, const std::string & groupName,
		IMContact & imContact) > imContactMovedEvent;

	/**
	 * Emitted when an IMContact has been added remotely.
	 *
	 * @param sender this class
	 * @param groupName the added group
	 */
	Event< void (IMContactList & sender, std::string groupName) > newContactGroupAddedEvent;

	/**
	 * Emitted when a group has been removed.
	 *
	 * @param sender this class
	 * @param groupName the removed group
	 */
	Event< void (IMContactList & sender, std::string groupName) > contactGroupRemovedEvent;

	IMContactListHandler(UserProfile & userProfile);

	~IMContactListHandler();

	/**
	 * Remotely adds an IMContact.
	 *
	 * @param groupName the group where we want to add the IMContact
	 * @param imContact the IMContact to add
	 */
	void addIMContact(const std::string & groupName, const IMContact & imContact);

	/**
	 * Remotely deletes an IMContact.
	 *
	 * @param groupName the group if the IMContact
	 * @param imContact the IMContact to remove
	 */
	void removeIMContact(const std::string & groupName, const IMContact & imContact);

	/**
	 * Remotely adds a group.
	 *
	 * @param groupName the group name to add the contact in
	 */
	void addGroup(const std::string & groupName);

	/**
	 * Remotely deletes a group.
	 *
	 * @param groupName the group name of the contact
	 */
	void removeGroup(const std::string & groupName);

	/**
	 * Renames a group.
	 *
	 * @param oldGroupName the group to rename
	 * @param newGroupName the desired group name
	 */
	void changeGroupName(const std::string & oldGroupName, const std::string & newGroupName);

	/**
	 * @see IMContactList::moveContactToGroup
	 */
	void moveContactToGroup(const std::string & newGroupName,
		const std::string & oldGroupName, const IMContact & imContact);

	/**
	 * Tells IMContactListHandler to unregister to all registered Events.
	 */
	void unregisterAllEvents();

private:

	/**
	 * @see IMContactList::newContactAddedEvent
	 */
	void newContactAddedEventHandler(IMContactList & sender,
		std::string groupName, std::string contactId);

	/**
	 * @see IMContactList::contactRemovedEvent
	 */
	void contactRemovedEventHandler(IMContactList & sender,
		std::string groupName, std::string contactId);

	/**
	 * @see IMContactList::contactMovedEvent
	 */
	void contactMovedEventHandler(IMContactList & sender,
		std::string groupName, std::string contactId);

	/**
	 * @see IMContactList::newContactGroupAddedEvent
	 */
	void newContactGroupAddedEventHandler(IMContactList & sender, std::string groupName);

	/**
	 * @see IMContactList::contactGroupRemovedEvent
	 */
	void contactGroupRemovedEventHandler(IMContactList & sender, std::string groupName);

	/**
	 * @see IMAccountManager::imAccountAddedEvent
	 */
	void imAccountAddedEventHandler(IMAccountManager & sender, std::string imAccountId);

	/**
	 * @see IMAccountManager::imAccountRemovedEvent
	 */
	void imAccountRemovedEventHandler(IMAccountManager & sender, std::string imAccountId);
	
	typedef std::map<std::string, IMContactList *> IMContactListMap;
	IMContactListMap _imContactListMap;

	typedef std::list<IMContact> MyIMContactList;
	MyIMContactList _imContactList;

	UserProfile & _userProfile;
};

#endif //IMCONTACTLISTHANDLER_H
