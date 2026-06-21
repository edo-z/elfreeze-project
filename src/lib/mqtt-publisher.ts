import mqtt from "mqtt";

let client: mqtt.MqttClient | null = null;

function getClient() {
  if (!client) {
    client = mqtt.connect("mqtt://localhost:1883");
    client.on("connect", () => console.log("[MQTT] Publisher connected"));
    client.on("error", (err) => console.error("[MQTT] Error", err));
  }
  return client;
}

export function publishThresholds(deviceId: string, warning: number, critical: number) {
  const c = getClient();
  const topic = `elfreeze/${deviceId}/config`;
  const payload = JSON.stringify({ warning, critical });
  console.log(`[MQTT] Publishing to ${topic}: ${payload}`);
  c.publish(topic, payload, { qos: 0, retain: true });
}
