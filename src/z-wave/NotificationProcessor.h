#pragma once

#include <map>

#include <Poco/Condition.h>
#include <Poco/Mutex.h>
#include <Poco/Nullable.h>
#include <Poco/SharedPtr.h>

#include <Notification.h>

#include "util/Loggable.h"
#include "zmq/ZMQClient.h"

namespace BeeeOn {

struct NodeInfo {
	bool m_polled;
	std::list<OpenZWave::ValueID> m_values;
};

typedef std::map<uint8_t, NodeInfo> nodeInfoMap;

/*
 * In OpenZWave, all feedback from the Z-Wave network is sent to the
 * application via callbacks. This class allows the application to add
 * a notification callback handler. All notifications will be reported to it.
 */
class NotificationProcessor : public Loggable {
public:
	NotificationProcessor();

	void lock();

	void waitUntilQueried();

	uint32_t homeID();

	void setZMQClient(Poco::SharedPtr<ZMQClient> client);

	/*
	 * Find data using notification
	 * @param &notification Provides a container for data sent via the notification
	 * @return Nullable Object, which contain object with information about node
	 */
	static Poco::Nullable<NodeInfo> findNodeInfo(OpenZWave::Notification const *notification);

	/*
	 * Find data for specific node using node id
	 * @param &nodeId Device identification for Z-Wave network
	 * @return Nullable Object, which contain object with information about node
	 */
	static Poco::Nullable<NodeInfo> findNodeInfo(const uint8 &nodeId);

	/*
	 * Driver failed to load
	 * @return True if driver failed to load
	 */
	bool initFailed() const;

	/*
	 * It handles notification from Z-Wave network.
	 * @param *notification Provides a container for data sent via the notification
	 */
	void onNotification(const OpenZWave::Notification *notification);

private:
	/*
	 * A new node value has been added to OpenZWave's list. These notifications
	 * occur after a node has been discovered.
	 * @param *notification Provides a container for data sent via the notification
	 */
	void valueAdded(const OpenZWave::Notification *noticiation);

	/*
	 * A node value has been updated from the Z-Wave and it is different
	 * from the previous value. It creates ZWaveMessage which contains
	 * specific method of product.
	 * @param *notification Provides a container for data sent via the notification
	 */
	void valueChanged(const OpenZWave::Notification *noticiation);

	/*
	 * A node value has been removed from OpenZWave's list.
	 * @param *notification Provides a container for data sent via the notification
	 */
	void valueRemoved(const OpenZWave::Notification *notification);

	/*
	 * A new node has been added to OpenZWave's list. This may be due to a
	 * device being added to the Z-Wave network, or because the application is
	 * initializing itself.
	 * @param *notification Provides a container for data sent via the notification
	 */
	void nodeAdded(const OpenZWave::Notification *notification);

	/*
	 * A node has been removed from OpenZWave's list. This may be due to a device
	 * being removed from the Z-Wave network, or because the application is closing.
	 * @param *notification Provides a container for data sent via the notification
	 */
	void nodeRemoved(const OpenZWave::Notification *notification);

private:
	Poco::Mutex m_lock;
	Poco::Mutex m_initMutex;
	Poco::Condition m_initCondition;
	static std::map<uint8_t, NodeInfo> m_nodesMap;
	Poco::SharedPtr<ZMQClient> m_zmqClient;

	uint32_t m_homeId;
	bool m_initFailed;
};

}

/*
 * Interface between Z-Wave network and NotificationProcesor
 * @param *notification Provides a container for data sent via the notification
 */
void onNotification(OpenZWave::Notification const *notification,
	void *context);
