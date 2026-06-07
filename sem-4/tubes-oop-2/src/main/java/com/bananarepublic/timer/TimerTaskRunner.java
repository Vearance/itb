package com.bananarepublic.timer;

import java.util.Objects;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledFuture;
import java.util.concurrent.TimeUnit;

public final class TimerTaskRunner {
    private final ScheduledExecutorService executorService;

    // create thread
    public TimerTaskRunner() {
        this.executorService = Executors.newSingleThreadScheduledExecutor(task -> {
            Thread thread = new Thread(task, "banana-republic-turn-timer");
            thread.setDaemon(true);
            return thread;
        });
    }

    public ScheduledFuture<?> scheduleAtFixedRate(Runnable task, long initialDelay, long period, TimeUnit unit) {
        Objects.requireNonNull(task, "task");
        Objects.requireNonNull(unit, "unit");
        return executorService.scheduleAtFixedRate(task, initialDelay, period, unit);
    }

    public void shutdown() {
        executorService.shutdownNow();
    }
}
