using System;
using System.Collections.Generic;
using System.Net.NetworkInformation;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using ReHargi.PosNetworkManager.Core;
using ReHargi.PosNetworkManager.Core.Interfaces;
using ReHargi.PosNetworkManager.Core.Models;

namespace ReHargi.PosNetworkManager.Network
{
    /// <summary>
    /// Service for reading and managing ARP table entries.
    /// Uses Windows IP Helper API.
    /// </summary>
    public class ArpService : IArpService
    {
        // P/Invoke declarations for Windows IP Helper API
        [DllImport("iphlpapi.dll", SetLastError = true)]
        private static extern int GetIpNetTable(IntPtr pIpNetTable, ref int pdwSize, bool bOrder);

        [DllImport("iphlpapi.dll", SetLastError = true)]
        private static extern int FlushIpNetTable(int dwIfIndex);

        [StructLayout(LayoutKind.Sequential)]
        public struct MIB_IPNETROW
        {
            [MarshalAs(UnmanagedType.U4)]
            public int dwIndex;
            [MarshalAs(UnmanagedType.U4)]
            public int dwPhysAddrLen;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
            public byte[] bPhysAddr;
            [MarshalAs(UnmanagedType.U4)]
            public int dwAddr;
            [MarshalAs(UnmanagedType.U4)]
            public int dwType;
        }

        [StructLayout(LayoutKind.Sequential)]
        public struct MIB_IPNETTABLE
        {
            [MarshalAs(UnmanagedType.U4)]
            public int dwNumEntries;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 1)]
            public MIB_IPNETROW[] table;
        }

        public async Task<IReadOnlyList<ArpEntry>> GetArpTableAsync(int interfaceIndex)
        {
            return await Task.Run(() =>
            {
                var entries = new List<ArpEntry>();

                try
                {
                    int size = 0;
                    IntPtr pIpNetTable = Marshal.AllocHGlobal(0);

                    // Get required buffer size
                    GetIpNetTable(pIpNetTable, ref size, false);

                    pIpNetTable = Marshal.ReAllocHGlobal(pIpNetTable, new IntPtr(size));
                    int ret = GetIpNetTable(pIpNetTable, ref size, false);

                    if (ret != 0) // ERROR_NO_DATA or other error
                    {
                        Marshal.FreeHGlobal(pIpNetTable);
                        return entries;
                    }

                    MIB_IPNETTABLE table = (MIB_IPNETTABLE)Marshal.PtrToStructure(pIpNetTable, typeof(MIB_IPNETTABLE));
                    IntPtr baseAddr = new IntPtr(pIpNetTable.ToInt64() + Marshal.SizeOf(table.dwNumEntries));

                    for (int i = 0; i < table.dwNumEntries; i++)
                    {
                        IntPtr addr = new IntPtr(baseAddr.ToInt64() + (i * Marshal.SizeOf(typeof(MIB_IPNETROW))));
                        MIB_IPNETROW row = (MIB_IPNETROW)Marshal.PtrToStructure(addr, typeof(MIB_IPNETROW));

                        // Filter by interface if specified
                        if (interfaceIndex > 0 && row.dwIndex != interfaceIndex)
                            continue;

                        string ipAddr = $"{row.dwAddr & 0xFF}.{(row.dwAddr >> 8) & 0xFF}.{(row.dwAddr >> 16) & 0xFF}.{(row.dwAddr >> 24) & 0xFF}";
                        string macAddr = "";
                        for (int j = 0; j < row.dwPhysAddrLen; j++)
                            macAddr += row.bPhysAddr[j].ToString("X2") + (j < row.dwPhysAddrLen - 1 ? "-" : "");

                        entries.Add(new ArpEntry
                        {
                            IpAddress = ipAddr,
                            MacAddress = macAddr,
                            Type = (ArpEntryType)row.dwType,
                            InterfaceIndex = row.dwIndex
                        });
                    }

                    Marshal.FreeHGlobal(pIpNetTable);
                }
                catch
                {
                    // If P/Invoke fails, return empty list (fallback to command-line parsing can be implemented)
                }

                return entries;
            });
        }

        public async Task<OperationResult> RefreshArpTableAsync()
        {
            return await Task.Run(() =>
            {
                try
                {
                    // Flush ARP table for all interfaces
                    FlushIpNetTable(-1);
                    return OperationResult.Success();
                }
                catch (Exception ex)
                {
                    return OperationResult.Failure($"ARP tablosu yenilenirken hata: {ex.Message}", null, ex.ToString());
                }
            });
        }

        public async Task<OperationResult> ClearArpEntryAsync(string ipAddress, int interfaceIndex)
        {
            return await Task.Run(() =>
            {
                try
                {
                    // TODO: Implement using arp -d command or direct API
                    return OperationResult.Success();
                }
                catch (Exception ex)
                {
                    return OperationResult.Failure($"ARP girişi temizlenirken hata: {ex.Message}", null, ex.ToString());
                }
            });
        }
    }
}
