package com.bananarepublic.exception.plugin;

import com.bananarepublic.exception.GameException;

public class PluginException extends GameException {
    public PluginException(String message) {
        super(message);
    }

    public PluginException(String message, Throwable cause) {
        super(message, cause);
    }
}
