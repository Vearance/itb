package com.bananarepublic.app;

import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

public final class ServiceRegistry {
    private final Map<Class<?>, Object> services = new HashMap<>();

    public <T> void register(Class<T> serviceType, T service) {
        services.put(
                Objects.requireNonNull(serviceType, "serviceType"),
                Objects.requireNonNull(service, "service")
        );
    }

    public <T> T get(Class<T> serviceType) {
        Object service = services.get(Objects.requireNonNull(serviceType, "serviceType"));
        if (service == null) {
            throw new IllegalStateException("Service is not registered: " + serviceType.getName());
        }
        return serviceType.cast(service);
    }

    public boolean contains(Class<?> serviceType) {
        return services.containsKey(Objects.requireNonNull(serviceType, "serviceType"));
    }

    public void unregister(Class<?> serviceType) {
        services.remove(Objects.requireNonNull(serviceType, "serviceType"));
    }
}
