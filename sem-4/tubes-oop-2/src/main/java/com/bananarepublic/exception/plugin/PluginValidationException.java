package com.bananarepublic.exception.plugin;

public class PluginValidationException extends PluginException {
    public PluginValidationException(String message) {
        super(message);
    }

    public PluginValidationException(String message, Throwable cause) {
        super(message, cause);
    }
}
