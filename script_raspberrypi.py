import paho.mqtt.client as mqtt 

from twilio.rest import Client 

 
##credentials deleted for personal security reasons
TWILIO_ACCOUNT_SID = "TWILIO_SID"

TWILIO_AUTH_TOKEN = "TWILIO_TOKEN" 

TWILIO_PHONE = "TWILIO PHONE" 

MY_PHONE = "PHONE NUMMBER" 

twilio_client = Client (TWILIO_ACCOUNT_SID, TWILIO_AUTH_TOKEN) 

 

MQTT_BROKER= "MQTT IP ADDRESS" 

MQTT_PORT = 1883 

MQTT_TOPIC = "Project" 

 

def on_connect(client, userdata, flags, rc): 

    print ("Connected to MQTT Broker") 

    client.subscribe(MQTT_TOPIC) 

    print ("Subscribed to topic") 

 

def on_message(client, userdata, msg): 

    message = msg.payload.decode() 

    print (f"Message received {message}") 

    if "ALARM > 1 MIN" in message: 

        try: 

            twilio_client.messages.create( 

                body = f"{message}", 

                from_ = TWILIO_PHONE, 

                to = MY_PHONE 

            ) 

            print ("SMS sent successfully") 

        except Exception as e: 

            print("Failed to send SMS:", e) 

             

client = mqtt.Client() 

client.on_connect = on_connect 

client.on_message = on_message 

client.connect(MQTT_BROKER, MQTT_PORT, 60) 

client.loop_forever() 