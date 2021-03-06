/**
 *  Copyright (C) 2014 3D Repo Ltd
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "repo_workerhistory.h"
#include <RepoNodeRevision>
#include <RepoGraphHistory>

//------------------------------------------------------------------------------
repo::gui::RepoWorkerHistory::RepoWorkerHistory(
    const repo::core::MongoClientWrapper &mongo,
    const QString &database)
	: mongo(mongo)
	, database(database.toStdString()) 
{}

//------------------------------------------------------------------------------

repo::gui::RepoWorkerHistory::~RepoWorkerHistory() {}

//------------------------------------------------------------------------------

void repo::gui::RepoWorkerHistory::run()
{
	if (!mongo.reconnect())
        std::cerr << tr("Connection failed").toStdString() << std::endl;
    else
    {
		mongo.reauthenticate(database);	

		std::list<std::string> fields;
		fields.push_back(REPO_NODE_LABEL_ID);
		fields.push_back(REPO_NODE_LABEL_SHARED_ID);
		fields.push_back(REPO_NODE_LABEL_MESSAGE);
		fields.push_back(REPO_NODE_LABEL_AUTHOR);
		fields.push_back(REPO_NODE_LABEL_TIMESTAMP);

        //----------------------------------------------------------------------
		// Retrieves all BSON objects until finished or cancelled.
		unsigned long long skip = 0;
		QDateTime datetime;
		std::auto_ptr<mongo::DBClientCursor> cursor;		
		do
		{
			for (; !cancelled && cursor.get() && cursor->more(); ++skip)
			{
                core::RepoNodeRevision revision(cursor->nextSafe());
				datetime.setMSecsSinceEpoch(revision.getTimestamp());
                //--------------------------------------------------------------
                emit revisionFetched(
                    QUuid(core::MongoClientWrapper::uuidToString(revision.getUniqueID()).c_str()),
                    QUuid(core::MongoClientWrapper::uuidToString(revision.getSharedID()).c_str()),
					QString::fromStdString(revision.getMessage()),
					QString::fromStdString(revision.getAuthor()),
					datetime);
			}
			if (!cancelled)
				cursor = mongo.listAllTailable(
					database, 
					REPO_COLLECTION_HISTORY, 
					fields, 
					REPO_NODE_LABEL_TIMESTAMP, 
					-1, 
					skip);		
		}
		while (!cancelled && cursor.get() && cursor->more());
	}
    //--------------------------------------------------------------------------
	emit RepoWorkerAbstract::finished();
}
