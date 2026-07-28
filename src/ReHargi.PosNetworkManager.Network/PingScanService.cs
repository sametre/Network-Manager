using System;
using System.Collections.Generic;
using System.Net;
using System.Net.NetworkInformation;
using System.Threading;
using System.Threading.Tasks;
using ReHargi.PosNetworkManager.Core.Models;

namespace ReHargi.PosNetworkManager.Network
{
    /// <summary>
    /// Service for performing asynchronous ping scans on subnet ranges.
    /// </summary>
    public class PingScanService
    {
        private readonly int _concurrencyLimit;
        private readonly int _timeoutMs;

        public event EventHandler<PingProgressEventArgs> ProgressChanged;

        public PingScanService(int concurrencyLimit = 20, int timeoutMs = 300)
        {
            _concurrencyLimit = concurrencyLimit;
            _timeoutMs = timeoutMs;
        }

        /// <summary>
        /// Scan a range of IP addresses and report which ones respond.
        /// </summary>
        public async Task<List<(string IpAddress, int LatencyMs)>> ScanRangeAsync(
            List<IPAddress> ipAddresses,
            CancellationToken cancellationToken)
        {
            var results = new List<(string, int)>();
            var semaphore = new SemaphoreSlim(_concurrencyLimit);
            var tasks = new List<Task>();

            for (int i = 0; i < ipAddresses.Count; i++)
            {
                var ip = ipAddresses[i];
                int index = i;

                tasks.Add(Task.Run(async () =>
                {
                    await semaphore.WaitAsync(cancellationToken);
                    try
                    {
                        var latency = await PingIpAsync(ip.ToString(), cancellationToken);
                        if (latency >= 0)
                        {
                            results.Add((ip.ToString(), latency));
                        }

                        ProgressChanged?.Invoke(this, new PingProgressEventArgs
                        {
                            CurrentIndex = index,
                            TotalCount = ipAddresses.Count,
                            CurrentIp = ip.ToString()
                        });
                    }
                    finally
                    {
                        semaphore.Release();
                    }
                }, cancellationToken));
            }

            await Task.WhenAll(tasks);
            return results;
        }

        /// <summary>
        /// Ping a single IP address.
        /// </summary>
        private async Task<int> PingIpAsync(string ipAddress, CancellationToken cancellationToken)
        {
            try
            {
                using (var ping = new Ping())
                {
                    var reply = await ping.SendPingAsync(ipAddress, _timeoutMs);
                    if (reply.Status == IPStatus.Success)
                    {
                        return (int)reply.RoundtripTime;
                    }
                }
            }
            catch
            {
                // Ignore ping failures
            }

            return -1;
        }
    }

    /// <summary>
    /// Event arguments for ping progress updates.
    /// </summary>
    public class PingProgressEventArgs : EventArgs
    {
        public int CurrentIndex { get; set; }
        public int TotalCount { get; set; }
        public string CurrentIp { get; set; }
    }
}
