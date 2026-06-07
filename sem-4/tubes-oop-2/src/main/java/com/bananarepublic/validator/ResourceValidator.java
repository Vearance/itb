package com.bananarepublic.validator;

import com.bananarepublic.core.resource.ResourceBank;
import com.bananarepublic.core.resource.ResourceType;

import java.util.Objects;

public final class ResourceValidator {
    private ResourceValidator() {
    }

    public static boolean canBankProduce(ResourceBank bank, ResourceType resourceType, int amount) {
        Objects.requireNonNull(bank, "bank");
        Objects.requireNonNull(resourceType, "resourceType");
        if (amount < 0) {
            throw new IllegalArgumentException("Production amount cannot be negative");
        }
        return bank.hasResource(resourceType, amount);
    }

    public static void validateBankCanProduce(ResourceBank bank, ResourceType resourceType, int amount) {
        if (!canBankProduce(bank, resourceType, amount)) {
            throw new IllegalStateException("Bank does not have enough " + resourceType + " to produce " + amount);
        }
    }
}
