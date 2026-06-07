package com.bananarepublic.core.resource;

import com.bananarepublic.core.player.AbstractPlayer;

import java.util.Objects;

public final class ResourceTransaction {
    private final AbstractPlayer sender;
    private final AbstractPlayer receiver;
    private final ResourceBundle resources;

    public ResourceTransaction(AbstractPlayer sender, AbstractPlayer receiver, ResourceBundle resources) {
        this.sender = Objects.requireNonNull(sender, "sender");
        this.receiver = Objects.requireNonNull(receiver, "receiver");
        this.resources = Objects.requireNonNull(resources, "resources").copy();
    }

    public void execute() {
        sender.getInventory().spendResourceBundle(resources);
        receiver.getInventory().addResourceBundle(resources);
    }

    public AbstractPlayer getSender() {
        return sender;
    }

    public AbstractPlayer getReceiver() {
        return receiver;
    }

    public ResourceBundle getResources() {
        return resources.copy();
    }
}
