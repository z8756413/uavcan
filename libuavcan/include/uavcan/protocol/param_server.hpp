/*
 * Copyright (C) 2014 Pavel Kirienko <pavel.kirienko@gmail.com>
 */

#ifndef UAVCAN_PROTOCOL_PARAM_SERVER_HPP_INCLUDED
#define UAVCAN_PROTOCOL_PARAM_SERVER_HPP_INCLUDED

#include <uavcan/protocol/param/GetSet.hpp>
#include <uavcan/protocol/param/ExecuteOpcode.hpp>
#include <uavcan/node/service_server.hpp>
#include <uavcan/util/method_binder.hpp>

namespace uavcan
{
/**
 * Implement this interface in the application to support the standard remote reconfiguration services.
 * Refer to @ref ParamServer.
 */
class UAVCAN_EXPORT IParamManager
{
public:
    typedef typename StorageType<typename protocol::param::GetSet::Response::FieldTypes::name>::Type Name;
    typedef typename StorageType<typename protocol::param::GetSet::Request::FieldTypes::index>::Type Index;
    typedef protocol::param::Value Value;
    typedef protocol::param::NumericValue NumericValue;

    virtual ~IParamManager() { }

    /**
     * Copy the parameter name to @ref out_name if it exists, otherwise do nothing.
     */
    virtual void getParamNameByIndex(Index index, Name& out_name) const = 0;

    /**
     * Assign by name if exists.
     */
    virtual void assignParamValue(const Name& name, const Value& value) = 0;

    /**
     * Read by name if exists, otherwise do nothing.
     */
    virtual void readParamValue(const Name& name, Value& out_value) const = 0;

    /**
     * Read param's default/max/min if available.
     * Note that min/max are only applicable to numeric params.
     * Implementation is optional.
     */
    virtual void readParamDefaultMaxMin(const Name& name, Value& out_default,
                                        NumericValue& out_max, NumericValue& out_min) const
    {
        (void)name;
        (void)out_default;
        (void)out_max;
        (void)out_min;
    }

    /**
     * Save all params to non-volatile storage.
     * @return Negative if failed.
     */
    virtual int saveAllParams() = 0;

    /**
     * Clear the non-volatile storage.
     * @return Negative if failed.
     */
    virtual int eraseAllParams() = 0;

    /**
     * Convenience methods that can be used to check if a param value is empty.
     */
    static bool isValueEmpty(const Value& val)
    {
        return val.value_bool.empty() &&
               val.value_int.empty() &&
               val.value_float.empty() &&
               val.value_string.empty();
    }
    static bool isValueEmpty(const NumericValue& val)
    {
        return val.value_int.empty() &&
               val.value_float.empty();
    }
};

/**
 * Convenience class for supporting the standard configuration services.
 * Highly recommended to use.
 */
class UAVCAN_EXPORT ParamServer
{
    typedef MethodBinder<ParamServer*, void (ParamServer::*)(const protocol::param::GetSet::Request&,
                                                             protocol::param::GetSet::Response&)> GetSetCallback;

    typedef MethodBinder<ParamServer*,
                         void (ParamServer::*)(const protocol::param::ExecuteOpcode::Request&,
                                               protocol::param::ExecuteOpcode::Response&)> ExecuteOpcodeCallback;

    ServiceServer<protocol::param::GetSet, GetSetCallback> get_set_srv_;
    ServiceServer<protocol::param::ExecuteOpcode, ExecuteOpcodeCallback> save_erase_srv_;
    IParamManager* manager_;

    void handleGetSet(const protocol::param::GetSet::Request& request, protocol::param::GetSet::Response& response);

    void handleExecuteOpcode(const protocol::param::ExecuteOpcode::Request& request,
                             protocol::param::ExecuteOpcode::Response& response);

public:
    explicit ParamServer(INode& node)
        : get_set_srv_(node)
        , save_erase_srv_(node)
        , manager_(NULL)
    { }

    /**
     * Starts the parameter server with given param manager instance.
     * Returns negative error code.
     */
    int start(IParamManager* manager);

    IParamManager* getParamManager() const { return manager_; }
};

}

#endif // UAVCAN_PROTOCOL_PARAM_SERVER_HPP_INCLUDED
