import requests

headers = {
    "Authorization": ""
}

url = "https://eu1.cloud.thethings.network/api/v3/as/applications/station-vigne/devices/eui-a8610a32344b7004/packages/storage/uplink_message"

response = requests.get(url, headers=headers)

print("Status code:", response.status_code)
print("Response content:")
print(response.text)  # Affiche le texte brut

try:
    data = response.json()
    print("Données JSON :", data)
except requests.exceptions.JSONDecodeError:
    print("❌ La réponse n’est pas un JSON valide.")
