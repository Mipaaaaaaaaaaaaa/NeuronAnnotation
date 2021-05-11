#include <DataBase.hpp>
#include <memory>

using Poco::Int64;

std::string DataBase::port("27017");
std::string DataBase::host("localhost");
std::string DataBase::name("NeuronAnnotation");
bool DataBase::isPoolRunning(false);
size_t DataBase::poolCapacity(16);
size_t DataBase::poolPeakCapacity(256);
DataBase::MongoDBConnectionFactoryPtr DataBase::g_connectionFactory;
DataBase::MongoDBConnectionPoolPtr DataBase::g_connectionPool;
Poco::MongoDB::Database DataBase::g_db(DataBase::name);

void DataBase::connect()
{
    std::cout << "Conneted to :" << host << ":" << port;
	g_connectionFactory.reset(new MongoDBConnectionFactory(host+":"+port));
	g_connectionPool.reset(new MongoDBConnectionPool(
	*g_connectionFactory, poolCapacity, poolPeakCapacity));
}

std::string DataBase::getName(){
    return name;
}

std::string DataBase::getHost(){
    return host;
}

std::string DataBase::getPort(){
    return port;
}

void DataBase::setName(const std::string &value){
    name = value;
}

void DataBase::setHost(const std::string &value){
    host = value;
}

void DataBase::setPort(const std::string &value){
    port = value;
}

bool DataBase::modifySWC(const NeuronSWC &swc, const std::string &tableName){
    return true;
}

bool DataBase::modifySWCs(const std::vector<std::shared_ptr<NeuronSWC> > &swcs, const std::string &tableName){
    return true;
}

bool DataBase::insertSWC(const NeuronSWC &swc, const std::string &tableName){
    if( findSWC(swc,tableName) ) return false;

    auto con = takeConnection();
    auto c = static_cast<Poco::MongoDB::Connection::Ptr>(con);
    Poco::MongoDB::Document::Ptr swcObj(new Poco::MongoDB::Document());
    swcObj->add("id",swc.id);
    swcObj->add("name",swc.name);
    swcObj->add("lind_id",swc.line_id);
    swcObj->add("x",swc.x);
    swcObj->add("y",swc.y);
    swcObj->add("z",swc.z);
    swcObj->add("parent",swc.pn);
    swcObj->add("color",swc.color);
    swcObj->add("user_id",swc.user_id);
    swcObj->add("timestamp",swc.timestamp);
    swcObj->add("seg_id",swc.seg_id);
    swcObj->add("seg_in_id",swc.seg_in_id);
    swcObj->add("seg_size",swc.seg_size);
    swcObj->add("type",int(swc.type));
    swcObj->add("radius",swc.radius);
    
    auto insert = g_db.createCommand();
    insert->selector().add("insert",tableName).add("document",swcObj);
    Poco::MongoDB::ResponseMessage response;
    c->sendRequest(*insert,response);
    auto doc = *(response.documents()[0]);
    return verifyResponse(doc);
}

bool DataBase::findSWC(const NeuronSWC &swc, const std::string &tableName){
    // take connection from pool
    auto con = takeConnection();
    auto c = static_cast<Poco::MongoDB::Connection::Ptr>(con);

    auto queryPtr = g_db.createQueryRequest(name+"."+tableName);
    queryPtr->selector().add("id", swc.id);

    // limit return numbers
    queryPtr->setNumberToReturn(1);
    Poco::MongoDB::ResponseMessage response;

    // send request to server
    c->sendRequest(*queryPtr, response);
    if (response.documents().empty()) {
        return false;
    }
    else return true;
}

bool DataBase::insertSWCs(const std::vector<std::shared_ptr<NeuronSWC> > &swcs, const std::string &tableName){
    Poco::MongoDB::Array::Ptr swcList(new Poco::MongoDB::Array());
    auto con = takeConnection();
    auto c = static_cast<Poco::MongoDB::Connection::Ptr>(con);
    int index = 0;
    for( auto swc = swcs.begin() ; swc != swcs.end() ; swc ++ ){
        if( findSWC(**swc,tableName) ) return false;

        Poco::MongoDB::Document::Ptr swcObj(new Poco::MongoDB::Document());
        swcObj->add("id",(*swc)->id);
        swcObj->add("name",(*swc)->name);
        swcObj->add("lind_id",(*swc)->line_id);
        swcObj->add("x",(*swc)->x);
        swcObj->add("y",(*swc)->y);
        swcObj->add("z",(*swc)->z);
        swcObj->add("parent",(*swc)->pn);
        swcObj->add("color",(*swc)->color);
        swcObj->add("user_id",(*swc)->user_id);
        swcObj->add("timestamp",(*swc)->timestamp);
        swcObj->add("seg_id",(*swc)->seg_id);
        swcObj->add("seg_in_id",(*swc)->seg_in_id);
        swcObj->add("seg_size",(*swc)->seg_size);
        swcObj->add("type",int((*swc)->type));
        swcObj->add("radius",(*swc)->radius);
        swcList->add(std::to_string(index++),swcObj);
    }

    auto insert = g_db.createCommand();

    insert->selector().add("insert",tableName).add("documents",swcList);

    Poco::MongoDB::ResponseMessage response;

    c->sendRequest(*insert,response);

    auto doc = *(response.documents()[0]);

    return verifyResponse(doc);
}

bool DataBase::deleteSWC(const NeuronSWC &swc, const std::string &tableName){
    // take connection
    auto con = takeConnection();
    auto c = static_cast<Poco::MongoDB::Connection::Ptr>(con);

    // create query fro finding book
    Poco::MongoDB::Document::Ptr query(new Poco::MongoDB::Document());
    query->add("id", swc.id);

    Poco::MongoDB::Document::Ptr del(new Poco::MongoDB::Document());
    del->add("q", query).add("limit", 1);

    Poco::MongoDB::Array::Ptr deletes(new Poco::MongoDB::Array());
    deletes->add(std::to_string(0), del);

    // create command
    auto deleteCmd = g_db.createCommand();
    deleteCmd->selector()
    .add("delete", tableName)
    .add("deletes", deletes);

    Poco::MongoDB::ResponseMessage response;
    c->sendRequest(*deleteCmd, response);
    auto doc = *(response.documents()[0]);
    return verifyResponse(doc);
    // for (auto i : response.documents()) {
    //     return i->toString(2);
    // }
    // return true;
}

bool DataBase::deleteSWCs(const std::vector<std::shared_ptr<NeuronSWC> > &swcs, const std::string &tableName){
    // take connection
    auto con = takeConnection();
    auto c = static_cast<Poco::MongoDB::Connection::Ptr>(con);
    int index = 0;
    Poco::MongoDB::Array::Ptr deletes(new Poco::MongoDB::Array());
    for ( auto swc = swcs.begin() ; swc != swcs.end() ; swc ++ ){
        // create query fro finding book
        Poco::MongoDB::Document::Ptr query(new Poco::MongoDB::Document());
        query->add("id", (*swc)->id);

        Poco::MongoDB::Document::Ptr del(new Poco::MongoDB::Document());
        del->add("q", query).add("limit", 1);
        deletes->add(std::to_string(index++), del);
    }
    // create command
    auto deleteCmd = g_db.createCommand();
    deleteCmd->selector()
    .add("delete", tableName)
    .add("deletes", deletes);

    Poco::MongoDB::ResponseMessage response;
    c->sendRequest(*deleteCmd, response);
    auto doc = *(response.documents()[0]);
    return verifyResponse(doc);

    return true;
}

std::string DataBase::getSWCFileStringFromTable(const std::string &tableName){
    // take connection from pool
    auto con = takeConnection();
    auto c = static_cast<Poco::MongoDB::Connection::Ptr>(con);

    auto queryPtr = g_db.createQueryRequest(tableName);
    queryPtr->selector();

    Poco::MongoDB::ResponseMessage response;

    // send request to server
    c->sendRequest(*queryPtr, response);
    if (response.documents().empty()) {
        return "";
    }
    // else return true;
    for( int i = 0 ; i < response.documents().size() ; i ++ ){
        std::cout << response.documents()[i] << endl;
    }
    return "";
}

bool DataBase::findTable(const std::string &tableName){
    // take connection from pool
    auto con = takeConnection();
    auto c = static_cast<Poco::MongoDB::Connection::Ptr>(con);

    auto queryPtr = g_db.createQueryRequest(tableName);
    queryPtr->selector();

    // limit return numbers
    queryPtr->setNumberToReturn(1);
    Poco::MongoDB::ResponseMessage response;

    // send request to server
    c->sendRequest(*queryPtr, response);
    if (response.documents().empty()) {
        return false;
    }
    else return true;
}

Poco::MongoDB::PooledConnection DataBase::takeConnection()
{
	static std::mutex connectionPoolLock;
	std::lock_guard<std::mutex> l(connectionPoolLock);

	Poco::MongoDB::PooledConnection pooledConnection(*g_connectionPool);
	auto c = static_cast<Poco::MongoDB::Connection::Ptr>(pooledConnection);

	if (!c) {
		// Connection pool can return null if the pool is full
		// TODO: Gracefully handle this here or implement
		// ObjectPool::borrowObjectWithTimeout
	}

	return std::move(pooledConnection);
}

bool DataBase::verifyResponse(const Poco::MongoDB::Document &response, bool expectOK){
	// TODO: Remove when updated MongoDB::Document header is used.
	auto &r = const_cast<Poco::MongoDB::Document &>(response);
	/*
	 *
http://docs.mongodb.org/manual/reference/command/insert/#insert-command-output
	 *
http://docs.mongodb.org/manual/reference/command/update/#update-command-output
	 * http://docs.mongodb.org/manual/reference/command/delete/
	 * http://docs.mongodb.org/manual/reference/command/findAndModify/
	 */

	std::ostringstream ostr;
	std::string responseJson;
    if (r.exists("ok")) {
        Poco::Int64 ok ;
        if (r.isType<Poco::Int32>("ok")) {
            ok = r.get<Poco::Int32>("ok");
        }
        else if (r.isType<Poco::Int64>("ok")) {
            ok = r.get<Poco::Int64>("ok");
        }
        else if (r.isType<double>("ok")) {
            ok = static_cast<double>(r.get<double>("ok"));
        }
        if (ok != 1) {
            ostr << "Command failed: ok = " << ok << ". ";
            // put response in string
            responseJson +=
            std::string("Command failed: ok = ") +
            std::to_string(ok) + std::string(". ");
            return false;
        }
        return true;
    }
    else if (expectOK) {
        ostr << "UNEXPECTED: Missing 'ok' in response.";

        // put response in string
        responseJson += "UNEXPECTED: Missing 'ok' in response.";
        return false;
    }
    else {
        // Document that does not have embedded status response,
        // e.g.from
        // find cursor
        return true;
    }
    std::cout << responseJson << std::endl;
	return false;
}