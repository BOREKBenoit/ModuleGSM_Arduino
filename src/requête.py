import requests

headers = {
    "Authorization": "Bearer NNSXS.GVQV5C2K7VVMXQMI6VYQ2QJ4CTNURVCZK2FR5II.LKXVDNOT6IWSS2K7UZ3AVGUBRVNUAOYC4V5CARXHB7Q3QTHZP6XA"
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
