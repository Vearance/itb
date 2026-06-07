package com.bananarepublic.app;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;

class ServiceRegistryTest {
    @Test
    void registersAndReturnsServiceByType() {
        ServiceRegistry registry = new ServiceRegistry();
        String value = "banana";

        registry.register(String.class, value);

        assertTrue(registry.contains(String.class));
        assertEquals(value, registry.get(String.class));
    }

    @Test
    void throwsWhenServiceIsMissing() {
        ServiceRegistry registry = new ServiceRegistry();

        assertThrows(IllegalStateException.class, () -> registry.get(String.class));
    }
}
