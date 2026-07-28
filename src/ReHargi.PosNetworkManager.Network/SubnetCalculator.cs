using System;
using System.Collections.Generic;
using System.Net;
using System.Threading.Tasks;

namespace ReHargi.PosNetworkManager.Network
{
    /// <summary>
    /// Helper class for subnet calculations.
    /// </summary>
    public static class SubnetCalculator
    {
        /// <summary>
        /// Calculate the network range from IP and subnet mask.
        /// </summary>
        public static (IPAddress NetworkAddress, IPAddress BroadcastAddress, List<IPAddress> UsableAddresses) CalculateRange(
            string ipAddress, string subnetMask)
        {
            try
            {
                var ip = IPAddress.Parse(ipAddress);
                var mask = IPAddress.Parse(subnetMask);

                long ipLong = BitConverter.ToInt32(ip.GetAddressBytes(), 0);
                long maskLong = BitConverter.ToInt32(mask.GetAddressBytes(), 0);

                long networkLong = ipLong & maskLong;
                long broadcastLong = networkLong | ~maskLong;

                byte[] networkBytes = BitConverter.GetBytes(networkLong);
                byte[] broadcastBytes = BitConverter.GetBytes(broadcastLong);

                var networkAddr = new IPAddress(networkBytes);
                var broadcastAddr = new IPAddress(broadcastBytes);

                var usable = new List<IPAddress>();
                for (long i = networkLong + 1; i < broadcastLong; i++)
                {
                    byte[] bytes = BitConverter.GetBytes(i);
                    usable.Add(new IPAddress(bytes));
                }

                return (networkAddr, broadcastAddr, usable);
            }
            catch
            {
                return (null, null, new List<IPAddress>());
            }
        }

        /// <summary>
        /// Check if an IP is within subnet range.
        /// </summary>
        public static bool IsIpInSubnet(string testIp, string networkIp, string subnetMask)
        {
            try
            {
                var testIpAddr = IPAddress.Parse(testIp);
                var networkAddr = IPAddress.Parse(networkIp);
                var maskAddr = IPAddress.Parse(subnetMask);

                long testLong = BitConverter.ToInt32(testIpAddr.GetAddressBytes(), 0);
                long networkLong = BitConverter.ToInt32(networkAddr.GetAddressBytes(), 0);
                long maskLong = BitConverter.ToInt32(maskAddr.GetAddressBytes(), 0);

                return (testLong & maskLong) == (networkLong & maskLong);
            }
            catch
            {
                return false;
            }
        }

        /// <summary>
        /// Get CIDR notation from subnet mask.
        /// </summary>
        public static int GetCidrNotation(string subnetMask)
        {
            try
            {
                var mask = IPAddress.Parse(subnetMask);
                long maskLong = BitConverter.ToInt32(mask.GetAddressBytes(), 0);
                int bits = 0;
                for (int i = 31; i >= 0; i--)
                {
                    if ((maskLong & (1L << i)) != 0)
                        bits++;
                }
                return bits;
            }
            catch
            {
                return 0;
            }
        }
    }
}
