## BACKGROUND

ZotbinsCore is an IoT (Internet of Things) "smart bin" which currently collects valuable waste metrics on the *University of California, Irvine* campus. A smart bin is a waste bin equipped with intelligent hardware—our smart bin is called the ZotBin.

ZotbinsCore collects waste metrics such as image data and bin usage, interfacing with the **[Zotbins App](https://apps.apple.com/us/app/zotbins/id6743295314)** to help users locate bins and properly dispose their trash. From an organizational standpoint, ZotbinsCore can also be extremely useful for analyzing waste footprint and exposing insightful trends that can help to correct improper practices.

Once the data is collected, it is relayed to our AWS IoT MQTT (Message Queuing Telemetry Transport) Broker which allows us to seamlessly connect an entire network of ZotBins to collect data across an entire campus.

The current iteration uses a dual ESP-32 system to support the large number of sensors and prioritize speed. We eventually plan to reduce the cost and footprint of the ZotBin.

## CHANGELOG

### VERSION 1.0

Currently ZotbinsCore collects the following metrics:
- Pictures of trash disposed in the ZotBin 
	- These images are categorized by Zotbins' Waste Recognition team
- Number of items disposed in the ZotBin
- Weight of items disposed in the ZotBin
- Fullness of the ZotBin
